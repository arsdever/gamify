#pragma once

namespace
{

template <typename T, size_t C, typename S = std::array<T, C>>
struct vertex_attribute
{
    using attribute_data_storage_type = S;
    using attribute_component_type = T;

    static constexpr size_t component_count = C;
    static constexpr size_t size = sizeof(T) * C;

    attribute_data_storage_type data;
};

template <typename... ATTRIBUTES>
struct calculate_combined_size_helper;

template <typename ATTRIBUTE0, typename... ATTRIBUTES>
struct calculate_combined_size_helper<ATTRIBUTE0, ATTRIBUTES...>
{
    static constexpr size_t size =
        calculate_combined_size_helper<ATTRIBUTES...>::size + ATTRIBUTE0::size;
};

template <typename ATTRIBUTE>
struct calculate_combined_size_helper<ATTRIBUTE>
{
    static constexpr size_t size = ATTRIBUTE::size;
};

template <typename... T>
struct reverse_order_tuple_helper;

template <typename... T>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<T>()...));

template <typename T, typename... U>
struct reverse_order_tuple_helper<T, U...>
{
    using type = tuple_cat_t<typename reverse_order_tuple_helper<U...>::type,
                             std::tuple<T>>;
};

template <typename T>
struct reverse_order_tuple_helper<T>
{
    using type = std::tuple<T>;
};

template <typename... T>
using reverse_order_tuple = reverse_order_tuple_helper<T...>::type;

} // namespace

using position_3d_attribute = vertex_attribute<float, 3, glm::vec3>;
using normal_3d_attribute = vertex_attribute<float, 3, glm::vec3>;
using uv_attribute = vertex_attribute<float, 2, glm::vec2>;
using color_attribute = vertex_attribute<float, 4, glm::vec4>;
using tangent_attribute = vertex_attribute<float, 3, glm::vec3>;
using bitangent_attribute = vertex_attribute<float, 3, glm::vec3>;
using position_2d_attribute = vertex_attribute<float, 2, glm::vec2>;

template <typename... ATTRIBUTES>
struct vertex
{
    using tuple_type = reverse_order_tuple<ATTRIBUTES...>;

    static constexpr size_t attribute_count = std::tuple_size_v<tuple_type>;
    static constexpr size_t size =
        calculate_combined_size_helper<ATTRIBUTES...>::size;
    static constexpr std::array<int, std::tuple_size_v<tuple_type>>
        attribute_component_counts = { ATTRIBUTES::component_count... };
    static constexpr std::array<int, std::tuple_size_v<tuple_type>>
        attribute_sizes = { ATTRIBUTES::size... };

    tuple_type _attributes;

    template <size_t I>
    std::tuple_element<attribute_count - I - 1,
                       tuple_type>::type::attribute_data_storage_type&
    get()
    {
        return std::get<attribute_count - I - 1>(_attributes).data;
    }

    static void activate_attributes()
    {
        for (int i = 0; i < attribute_count; ++i)
        {
            glEnableVertexAttribArray(i);
        }
    }

    static void initialize_attributes()
    {
        size_t attribute_offset = 0;
        for (int i = 0; i < attribute_count; ++i)
        {
            glVertexAttribPointer(i,
                                  attribute_component_counts[ i ],
                                  0x1406,
                                  0,
                                  size,
                                  (void*)attribute_offset);
            attribute_offset += attribute_sizes[ i ];
        }
    }
};

struct vertex3d
    : vertex<position_3d_attribute,
             normal_3d_attribute,
             uv_attribute,
             color_attribute,
             tangent_attribute,
             bitangent_attribute>
{
    vertex3d() = default;

    vertex3d(position_3d_attribute::attribute_data_storage_type pos)
    {
        position() = pos;
    }

    vertex3d(position_3d_attribute::attribute_component_type x,
             position_3d_attribute::attribute_component_type y,
             position_3d_attribute::attribute_component_type z)
    {
        position() = { x, y, z };
    }

    position_3d_attribute::attribute_data_storage_type& position()
    {
        return get<0>();
    }
    normal_3d_attribute::attribute_data_storage_type& normal()
    {
        return get<1>();
    }
    uv_attribute::attribute_data_storage_type& uv() { return get<2>(); }
    color_attribute::attribute_data_storage_type& color() { return get<3>(); }
    tangent_attribute::attribute_data_storage_type& tangent()
    {
        return get<4>();
    }
    bitangent_attribute::attribute_data_storage_type& bitangent()
    {
        return get<5>();
    }

    inline void
    set_position(position_3d_attribute::attribute_data_storage_type pos)
    {
        position() = pos;
    }
    inline void
    set_normal(normal_3d_attribute::attribute_data_storage_type norm)
    {
        normal() = norm;
    }
    inline void set_uv(uv_attribute::attribute_data_storage_type coord)
    {
        uv() = coord;
    }
    inline void set_color(color_attribute::attribute_data_storage_type col)
    {
        color() = col;
    }
    inline void set_tangent(tangent_attribute::attribute_data_storage_type tan)
    {
        tangent() = tan;
    }
    inline void
    set_bitangent(bitangent_attribute::attribute_data_storage_type bitan)
    {
        bitangent() = bitan;
    }

    inline position_3d_attribute::attribute_data_storage_type get_position()
    {
        return position();
    }

    inline normal_3d_attribute::attribute_data_storage_type get_normal()
    {
        return normal();
    }

    inline uv_attribute::attribute_data_storage_type get_uv() { return uv(); }

    inline color_attribute::attribute_data_storage_type get_color()
    {
        return color();
    }

    inline tangent_attribute::attribute_data_storage_type get_tangent()
    {
        return tangent();
    }

    inline bitangent_attribute::attribute_data_storage_type get_bitangent()
    {
        return bitangent();
    }
};

struct vertex2d : vertex<position_2d_attribute, uv_attribute>
{
    position_2d_attribute::attribute_data_storage_type& position()
    {
        return get<0>();
    }
    uv_attribute::attribute_data_storage_type& uv() { return get<1>(); }
};

struct simple_vertex2d : vertex<position_2d_attribute>
{
    position_2d_attribute::attribute_data_storage_type& position()
    {
        return get<0>();
    }
};

struct simple_vertex3d : vertex<position_3d_attribute>
{
    position_3d_attribute::attribute_data_storage_type& position()
    {
        return get<0>();
    }
};

struct colored_vertex2d : vertex<position_2d_attribute, color_attribute>
{
    position_2d_attribute::attribute_data_storage_type& position()
    {
        return get<0>();
    }

    color_attribute::attribute_data_storage_type& color() { return get<1>(); }
};
