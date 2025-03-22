#pragma once

#include <jpeglib.h>
#include <setjmp.h>
#include <turbojpeg.h>

#include "common/file.hpp"
#include "graphics/texture.hpp"

namespace graphics
{
class jpg
{
public:
    static jpg load(common::file& f)
    {
        JSAMPARRAY buffer; /* Output row buffer */
        int row_stride;    /* physical row width in output buffer */

        auto data = f.read_all<std::vector<unsigned char>>();
        f.close();
        jpg result { std::move(data) };

        if (result._data.empty())
        {
            return result;
        }

        result._cinfo.err = jpeg_std_error(&result._err.pub);
        result._err.pub.error_exit = jpeg_error_exit;

        if (setjmp(result._err.setjmp_buffer))
        {
            jpeg_destroy_decompress(&result._cinfo);
            return result;
        }

        jpeg_create_decompress(&result._cinfo);
        jpeg_mem_src(&result._cinfo, result._data.data(), result._data.size());

        if (result._cinfo.jpeg_color_space != JCS_RGB ||
            result._cinfo.jpeg_color_space != JCS_GRAYSCALE)
            result._cinfo.out_color_space = JCS_RGB;

        (void)jpeg_read_header(&result._cinfo, TRUE);
        (void)jpeg_start_decompress(&result._cinfo);
        row_stride =
            result._cinfo.output_width * result._cinfo.output_components;
        buffer = (*result._cinfo.mem->alloc_sarray)(
            (j_common_ptr)&result._cinfo, JPOOL_IMAGE, row_stride, 1);

        metadata& md = result._metadata;
        md._width = result._cinfo.output_width;
        md._height = result._cinfo.output_height;
        md._channel_count = result._cinfo.out_color_components;
        md._bits_per_pixel = result._cinfo.out_color_space == JCS_RGB ? 24 : 8;
        md._bytes_per_row =
            result._cinfo.output_width * result._cinfo.output_components;
        switch (result._cinfo.out_color_space)
        {
        case JCS_GRAYSCALE:
        {
            md._color_type = graphics::texture::format::GRAYSCALE;
            break;
        }
        case JCS_RGB:
        {
            md._color_type = graphics::texture::format::RGB;
            break;
        }
        default:
        {
            md._color_type = graphics::texture::format::UNSPECIFIED;
            break;
        }
        }
        return result;
    }

    ~jpg()
    {
        (void)jpeg_finish_decompress(&_cinfo);
        jpeg_destroy_decompress(&_cinfo);
    }

    inline auto get_width() { return _metadata._width; }

    inline auto get_height() { return _metadata._height; }

    inline auto get_format() { return _metadata._color_type; }

    size_t read_pixels(unsigned char* data)
    {
        auto row_stride = _cinfo.output_width * _cinfo.output_components;
        auto data_size = _cinfo.output_height * row_stride;
        if (data == nullptr)
            return data_size;

        std::vector<unsigned char*> buffer(_cinfo.output_height);
        for (int i = 0; i < _cinfo.output_height; ++i)
        {
            buffer[ i ] = data + i * row_stride;
        }

        size_t count = 0;
        for (size_t i = 0; i < _cinfo.output_height; i += count)
        {
            count = jpeg_read_scanlines(
                &_cinfo, buffer.data() + i, _cinfo.output_height - i);
        }

        return data_size;
    }

    struct error_msg
    {
        struct jpeg_error_mgr pub; /* "public" fields */

        jmp_buf setjmp_buffer; /* for return to caller */
    };

    typedef struct error_msg* error_msg_ptr;

    static void jpeg_error_exit(j_common_ptr cinfo)
    {
        error_msg_ptr myerr = (error_msg_ptr)cinfo->err;
        (*cinfo->err->output_message)(cinfo);
        longjmp(myerr->setjmp_buffer, 1);
    }

private:
    jpg(std::vector<unsigned char> data)
        : _data(std::move(data))
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

    std::vector<unsigned char> _data;
    jpeg_decompress_struct _cinfo;
    error_msg _err;
};
} // namespace graphics
