#pragma once

#include <png.h>

#include "common/file.hpp"
#include "graphics/texture.hpp"

namespace graphics
{
class png
{
public:
    static png load(common::file& f)
    {
        png_structp png =
            png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png)
            return { f, nullptr, nullptr };

        auto reader = [](png_structp png, png_bytep data, png_size_t length)
        {
            common::file* fp =
                reinterpret_cast<common::file*>(png_get_io_ptr(png));

            if (length > 0)
            {
                fp->read(reinterpret_cast<char*>(data), length);
            }
        };
        png_set_read_fn(png, &f, reader);

        png_infop info = png_create_info_struct(png);
        if (!info)
            return { f, png, nullptr };

        class png result { f, png, info };
        if (setjmp(png_jmpbuf(png)))
            return result;

        png_read_info(png, info);

        auto width = png_get_image_width(png, info);
        auto height = png_get_image_height(png, info);
        auto color_type = png_get_color_type(png, info);
        auto bit_depth = png_get_bit_depth(png, info);

        // For simplicity let the modifications when loading png images
        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);
        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png);
        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);
        png_read_update_info(png, info);

        width = png_get_image_width(png, info);
        height = png_get_image_height(png, info);
        color_type = png_get_color_type(png, info);
        bit_depth = png_get_bit_depth(png, info);

        result._metadata._width = width;
        result._metadata._height = height;

        switch (color_type)
        /* Color type covnersion is done based on the following table
            0       1,2,4,8,16  Each pixel is a grayscale sample.
            2       8,16        Each pixel is an R,G,B triple.
            3       1,2,4,8     Each pixel is a palette index;
                                a PLTE chunk must appear.
            4       8,16        Each pixel is a grayscale sample,
                                followed by an alpha sample.
            6       8,16        Each pixel is an R,G,B triple,
                                followed by an alpha sample.
            http://www.libresult.org/pub/png/spec/1.2/PNG-Chunks.html
        */
        {
        case 0:
        {
            result._metadata._color_type = graphics::texture::format::GRAYSCALE;
            result._metadata._channel_count = 1;
            break;
        }
        case 2:
        {
            result._metadata._color_type = graphics::texture::format::RGB;
            result._metadata._channel_count = 3;
            break;
        }
        case 3:
        {
            result._metadata._color_type =
                graphics::texture::format::UNSPECIFIED;
            result._metadata._channel_count = 3;
            break;
        }
        case 4:
        {
            result._metadata._color_type =
                graphics::texture::format::UNSPECIFIED;
            result._metadata._channel_count = 2;
            break;
        }
        case 6:
        {
            result._metadata._color_type = graphics::texture::format::RGBA;
            result._metadata._channel_count = 4;
            break;
        }
        default:
        {
            result._metadata._color_type =
                graphics::texture::format::UNSPECIFIED;
            result._metadata._channel_count = 0;
            break;
        }
        }

        result._metadata._bits_per_pixel =
            bit_depth * result._metadata._channel_count;
        result._metadata._bytes_per_row = png_get_rowbytes(png, info);
        return result;
    }

    ~png() { png_destroy_read_struct(&handle, &info, NULL); }

    inline auto get_width() { return _metadata._width; }

    inline auto get_height() { return _metadata._height; }

    inline auto get_format() { return _metadata._color_type; }

    template <typename T>
        requires requires {
            std::is_same_v<T, char> || std::is_same_v<T, unsigned char>;
        }
    size_t read_pixels(T* data)
    {
        auto data_size = get_height() * png_get_rowbytes(handle, info);
        if (data == nullptr)
            return data_size;

        for (int i = 0; i < get_height(); ++i)
        {
            png_read_row(handle,
                         reinterpret_cast<png_bytep>(
                             data + i * png_get_rowbytes(handle, info)),
                         nullptr);
        }
        return data_size;
    }

private:
    png(common::file& f, png_structp handle, png_infop info)
        : file(f)
        , handle(handle)
        , info(info)
    {
    }

    struct metadata
    {
        size_t _width;
        size_t _height;
        size_t _bits_per_pixel;
        size_t _bytes_per_row;
        unsigned char _channel_count;
        graphics::texture::format _color_type;
    } _metadata;

    common::file& file;
    png_structp handle;
    png_infop info;
};
} // namespace graphics
