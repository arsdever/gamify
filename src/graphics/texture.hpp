#pragma once

#include "common/common_fwd.hpp"

namespace graphics
{
class texture : public std::enable_shared_from_this<graphics::texture>
{
public:
    enum class format : char
    {
        UNSPECIFIED,
        GRAYSCALE,
        DEPTH,
        RGB,
        RGBA,
    };

    enum class sampling_mode
    {
        nearest,
        linear,
        nearest_nearest,
        nearest_linear,
        linear_nearest,
        linear_linear,
    };

    enum class wrapping_mode
    {
        repeat,
        mirrored_repeat,
        clamp_to_edge,
        clamp_to_border,
    };

public:
    texture();
    texture(const texture& other) = delete;
    texture(texture&& other);
    texture& operator=(const texture& other) = delete;
    texture& operator=(texture&& other);
    ~texture();

    void init(size_t width, size_t height, format texture_format = format::RGB);

    void set_samples(int sample_count);
    int get_samples();

    glm::uvec2 get_size() const;
    size_t get_width() const;
    size_t get_height() const;
    size_t get_channel_count() const;
    template <typename T>
    const T* raw_data() const
    {
        return reinterpret_cast<const T*>(_data_buffer.data());
    }
    template <typename T>
    T* raw_data()
    {
        return reinterpret_cast<T*>(_data_buffer.data());
    }
    void get_data(char* data_ptr);
    void set_data(std::vector<char> data);
    void set_data(const char* data_ptr);
    void set_rect_data(glm::vec<2, size_t> pos,
                       glm::vec<2, size_t> size,
                       const char* data_ptr);
    void bind() const;
    void unbind() const;
    void set_active_texture(size_t index) const;
    void set_sampling_mode_min(sampling_mode mode);
    sampling_mode get_sampling_mode_min() const;
    void set_sampling_mode_mag(sampling_mode mode);
    sampling_mode get_sampling_mode_mag() const;
    void set_wrapping_mode(bool x, bool y, wrapping_mode mode);
    wrapping_mode get_wrapping_mode_x() const;
    wrapping_mode get_wrapping_mode_y() const;

    void clone(const texture* other_texture);

    unsigned native_id() const;

    static void static_bind(size_t id, bool ms);
    static void static_unbind(bool ms);

    void set_contents_from_file(common::file& file);
    void set_contents_from_file(std::string_view path);
    static std::shared_ptr<graphics::texture> from_file(std::string_view path);
    static std::shared_ptr<graphics::texture> from_file(common::file& file);

private:
    static int convert_to_gl_internal_format(format f);
    static int convert_to_gl_format(format f);
    static size_t pixel_component_count(format f);
    unsigned target() const;

    void fetch_from_gpu();

    void set_contents_from_png_file(common::file& file);
    void set_contents_from_jpg_file(common::file& file);
    static std::shared_ptr<graphics::texture> from_png_file(common::file& file);
    static std::shared_ptr<graphics::texture> from_jpg_file(common::file& file);

private:
    size_t _width { 0 };
    size_t _height { 0 };
    unsigned _texture_id { 0 };
    format _format { format::UNSPECIFIED };
    int _samples { 1 };
    std::vector<char> _data_buffer;

public:
    // TODO: may not be the best place for this object
    static std::vector<texture*> _textures;
};
} // namespace graphics
