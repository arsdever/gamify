#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/vec3.hpp>

#include "asset_loaders/fbx.hpp"

#include "asset_manager.hpp"
#include "assimp/quaternion.h"
#include "camera.hpp"
#include "components/mesh_component.hpp"
#include "components/mesh_renderer_component.hpp"
#include "feature_flags.hpp"
#include "game_object.hpp"
#include "light.hpp"
#include "logging.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "scene.hpp"

glm::vec3 convert(aiVector3D ai_vec3)
{
    return { ai_vec3.x, ai_vec3.y, ai_vec3.z };
}

glm::quat convert(aiQuaternion ai_quat)
{
    return { ai_quat.w, ai_quat.x, ai_quat.y, ai_quat.z };
}

glm::vec3 convert(aiColor3D ai_quat)
{
    return { ai_quat.r, ai_quat.g, ai_quat.b };
}

void asset_loader_FBX::load(std::string_view path)
{
    Assimp::Importer importer;
    const aiScene* ai_scene =
        importer.ReadFile(path.data(),
                          aiProcess_CalcTangentSpace | aiProcess_Triangulate |
                              aiProcess_JoinIdenticalVertices |
                              aiProcess_SortByPType | aiProcess_EmbedTextures);

    scene* s = nullptr;
    if (feature_flags::get_flag(feature_flags::flag_name::load_fbx_as_scene))
    {
        s = new scene;
    }

    auto log = get_logger("fbx_loader");
    std::queue<aiNode*> dfs_queue;
    dfs_queue.push(ai_scene->mRootNode);
    while (!dfs_queue.empty())
    {
        aiNode* node = dfs_queue.front();
        dfs_queue.pop();
        for (int i = 0; i < node->mNumChildren; ++i)
        {
            dfs_queue.push(node->mChildren[ i ]);
        }

        log->info("Node: {} meshes: {}", node->mName.C_Str(), node->mNumMeshes);
        if (node->mNumMeshes > 0)
        {
            game_object* obj = new game_object;
            mesh* m = nullptr;
            {
                std::vector<const aiMesh*> ai_submeshes;
                for (int i = 0; i < node->mNumMeshes; ++i)
                {
                    ai_submeshes.push_back(
                        ai_scene->mMeshes[ node->mMeshes[ i ] ]);
                }

                m = load_mesh(std::move(ai_submeshes));
                asset_manager::default_asset_manager()->register_asset(
                    std::format("{}_mesh", node->mName.C_Str()), m);
            }
            material* mat = new material;
            mat->set_shader_program(
                asset_manager::default_asset_manager()->get_shader("standard"));
            mat->declare_property("u_albedo_texture_strength",
                                  material_property::data_type::type_float);
            mat->declare_property(
                "u_albedo_color",
                material_property::data_type::type_float_vector_4);
            mat->declare_property("u_normal_texture_strength",
                                  material_property::data_type::type_float);
            mat->declare_property("u_model_matrix",
                                  material_property::data_type::unknown);
            mat->declare_property("u_vp_matrix",
                                  material_property::data_type::unknown);
            mat->set_property_value("u_albedo_texture_strength", 0.0f);
            mat->set_property_value(
                "u_albedo_color", 0.8f, 0.353f, 0.088f, 1.0f);
            mat->set_property_value("u_normal_texture_strength", 0.0f);

            obj->create_component<mesh_renderer_component>()->set_material(mat);
            obj->create_component<mesh_component>()->set_mesh(m);
            obj->set_name(node->mName.C_Str());
            if (s)
            {
                s->add_object(obj);
            }
        }
    }

    for (int i = 0; i < ai_scene->mNumCameras; ++i)
    {
        auto cam = load_camera(ai_scene->mCameras[ i ]);
        const auto& ai_camera = *ai_scene->mCameras[ i ];
        const auto& ai_camera_node =
            *ai_scene->mRootNode->FindNode(ai_camera.mName);
        aiMatrix4x4 camMat;
        ai_camera.GetCameraMatrix(camMat);
        aiMatrix4x4 final = ai_camera_node.mTransformation;
        final *= camMat;
        aiVector3D pos;
        aiVector3D scale;
        aiQuaternion rot;
        final.Decompose(scale, rot, pos);
        auto& t = cam->get_transform();
        t.set_position(convert(pos) / convert(scale));
        t.set_rotation(convert(rot));
        log->info("Camera: {}", ai_camera.mName.C_Str());
    }

    for (int i = 0; i < ai_scene->mNumLights; ++i)
    {
        auto l = new light;
        const auto& ai_light = *ai_scene->mLights[ i ];
        const auto& ai_light_node =
            *ai_scene->mRootNode->FindNode(ai_light.mName);
        aiMatrix4x4 mat = ai_light_node.mTransformation;
        aiVector3D pos;
        aiVector3D scale;
        aiQuaternion rot;
        mat.Decompose(scale, rot, pos);
        l->get_transform().set_position(convert(ai_light.mPosition + pos) /
                                        100.0f);
        glm::vec3 combined_intensity_color = convert(ai_light.mColorDiffuse);
        auto intensity = std::max(
            std::max(combined_intensity_color.x, combined_intensity_color.y),
            combined_intensity_color.z);
        l->set_color(combined_intensity_color / intensity);
        l->set_intensity(intensity / 100);
    }
}

mesh* asset_loader_FBX::load_mesh(std::vector<const aiMesh*> ai_submeshes)
{
    mesh* result;
    std::vector<vertex3d> vertices;
    std::vector<int> indices;
    std::vector<mesh::submesh_info> submeshes;

    for (auto ai_mesh : ai_submeshes)
    {
        mesh::submesh_info info;
        info.material_index = ai_mesh->mMaterialIndex;
        info.vertex_index_offset = indices.size();

        for (int vertex_index = 0; vertex_index < ai_mesh->mNumVertices;
             ++vertex_index)
        {
            vertices.push_back({});
            constexpr char position_name[] = "position";
            vertices.back().position() = {
                ai_mesh->mVertices[ vertex_index ].x,
                ai_mesh->mVertices[ vertex_index ].y,
                ai_mesh->mVertices[ vertex_index ].z
            };
            vertices.back().normal() = { ai_mesh->mNormals[ vertex_index ].x,
                                         ai_mesh->mNormals[ vertex_index ].y,
                                         ai_mesh->mNormals[ vertex_index ].z };
            vertices.back().uv() = {
                ai_mesh->mTextureCoords[ 0 ][ vertex_index ].x,
                ai_mesh->mTextureCoords[ 0 ][ vertex_index ].y,
            };
        }

        for (int face_index = 0; face_index < ai_mesh->mNumFaces; ++face_index)
        {
            const aiFace& assimp_face = ai_mesh->mFaces[ face_index ];
            for (int j = 0; j < assimp_face.mNumIndices; ++j)
                indices.push_back(assimp_face.mIndices[ j ] +
                                  info.vertex_index_offset);
        }

        submeshes.push_back(std::move(info));
    }

    result = new mesh;
    result->set_vertices(std::move(vertices));
    result->set_indices(std::move(indices));
    result->set_submeshes(std::move(submeshes));
    result->init();

    return result;
}

camera* asset_loader_FBX::load_camera(const aiCamera* ai_camera)
{
    camera* result = new camera;
    result->set_fov(ai_camera->mHorizontalFOV * 2.0f);
    result->set_ortho(false);
    return result;
}

light* asset_loader_FBX::load_light(const aiLight* ai_light)
{
    light* result = new light;
    return result;
}
