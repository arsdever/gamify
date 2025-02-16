#pragma once

class graphics_buffer
{
public:
    enum class type
    {
        vertex,
        index,
        shader_storage,
    };

    enum class usage_type
    {
        stream_draw,
        stream_read,
        stream_copy,
        static_draw,
        static_read,
        static_copy,
        dynamic_draw,
        dynamic_read,
        dynamic_copy,
    };

public:
    graphics_buffer(type type);
    graphics_buffer(graphics_buffer&& o);
    graphics_buffer(const graphics_buffer& o) = delete;
    graphics_buffer& operator=(graphics_buffer&& o);
    graphics_buffer& operator=(const graphics_buffer& o) = delete;
    ~graphics_buffer();

    void set_data(void* data_buffer);
    void get_data(void* data_buffer) const;

    void set_element_count(int element_count);
    int get_element_count() const;
    void set_element_stride(int element_stride);
    int get_element_stride() const;
    void set_usage_type(usage_type usage_type);
    usage_type get_usage_type() const;
    unsigned get_handle() const;
    void bind(int binding_id);

    void release();

private:
    type _type { type::vertex };
    usage_type _usage_type { usage_type::static_draw };
    unsigned _handle { 0 };
    int _element_count { 0 };
    int _element_stride { 0 };
};
