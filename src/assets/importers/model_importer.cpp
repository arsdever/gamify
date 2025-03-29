#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "assets/importers/model_importer.hpp"

#include "common/file_lock.hpp"
#include "common/logging.hpp"
#include "graphics/mesh.hpp"

namespace
{
logger log() { return get_logger("asset_manager"); }
} // namespace

namespace assets
{
void model_importer::initialize_asset(asset& ast)
{
    ast.get_raw_data() = std::make_shared<graphics::mesh>();
}

void model_importer::read_asset_data(std::string_view asset_path)
{
    auto content = common::file::read_all<std::vector<char>>(asset_path);
    auto path = common::filesystem::path(asset_path);
    Assimp::Importer importer;
    const aiScene* ai_scene = importer.ReadFileFromMemory(
        content.data(),
        content.size(),
        aiProcess_CalcTangentSpace | aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
            aiProcess_EmbedTextures,
        path.extension().data());

    if (!ai_scene)
    {
        log()->error("Failed to load: {}", asset_path);
        return;
    }

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

        log()->debug(
            "Node: {} meshes: {}", node->mName.C_Str(), node->mNumMeshes);
        if (node->mNumMeshes > 0)
        {
            {
                std::vector<const aiMesh*> ai_submeshes;
                for (int i = 0; i < node->mNumMeshes; ++i)
                {
                    ai_submeshes.push_back(
                        ai_scene->mMeshes[ node->mMeshes[ i ] ]);
                }

                std::vector<vertex3d> vertices;
                std::vector<int> indices;
                std::vector<graphics::mesh::submesh_info> submeshes;

                for (auto ai_mesh : ai_submeshes)
                {
                    graphics::mesh::submesh_info info;
                    size_t prev_vertex_count = vertices.size();
                    info.material_index = ai_mesh->mMaterialIndex;
                    info.vertex_index_offset = indices.size();

                    for (int vertex_index = 0;
                         vertex_index < ai_mesh->mNumVertices;
                         ++vertex_index)
                    {
                        vertices.push_back({});
                        constexpr char position_name[] = "position";
                        vertices.back().position() = {
                            ai_mesh->mVertices[ vertex_index ].x,
                            ai_mesh->mVertices[ vertex_index ].y,
                            ai_mesh->mVertices[ vertex_index ].z
                        };
                        vertices.back().normal() = {
                            ai_mesh->mNormals[ vertex_index ].x,
                            ai_mesh->mNormals[ vertex_index ].y,
                            ai_mesh->mNormals[ vertex_index ].z
                        };
                        vertices.back().uv() = {
                            ai_mesh->mTextureCoords[ 0 ][ vertex_index ].x,
                            ai_mesh->mTextureCoords[ 0 ][ vertex_index ].y,
                        };

                        vertices.back().tangent() = {
                            ai_mesh->mTangents[ vertex_index ].x,
                            ai_mesh->mTangents[ vertex_index ].y,
                            ai_mesh->mTangents[ vertex_index ].z
                        };

                        vertices.back().bitangent() = {
                            ai_mesh->mBitangents[ vertex_index ].x,
                            ai_mesh->mBitangents[ vertex_index ].y,
                            ai_mesh->mBitangents[ vertex_index ].z
                        };
                    }

                    for (int face_index = 0; face_index < ai_mesh->mNumFaces;
                         ++face_index)
                    {
                        const aiFace& assimp_face =
                            ai_mesh->mFaces[ face_index ];
                        for (int j = 0; j < assimp_face.mNumIndices; ++j)
                            indices.push_back(assimp_face.mIndices[ j ] +
                                              prev_vertex_count);
                    }

                    info.vertex_count = vertices.size() - prev_vertex_count;
                    info.index_count =
                        indices.size() - info.vertex_index_offset;
                    submeshes.push_back(std::move(info));
                }

                _data->set_vertices(std::move(vertices));
                _data->set_indices(std::move(indices));
                _data->set_submeshes(std::move(submeshes));
                _data->init();
            }
        }
    }

    for (int i = 0; i < ai_scene->mNumMaterials; ++i)
    {
        const aiMaterial* material = ai_scene->mMaterials[ i ];

        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        log()->debug("Material: {}", name.C_Str());
    }
}
} // namespace assets
