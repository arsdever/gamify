/* clang-format off */
#include <stdio.h>
/* clang-format on */

#include <jpeglib.h>
#include <png.h>
#include <setjmp.h>
#include <turbojpeg.h>

#include "asset_loaders/jpg.hpp"

#include "graphics/image.hpp"

struct my_error_mgr
{
    struct jpeg_error_mgr pub; /* "public" fields */

    jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

void asset_loader_JPG::load(std::string_view path)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE* infile;      /* source file */
    JSAMPARRAY buffer; /* Output row buffer */
    int row_stride;    /* physical row width in output buffer */

    if ((infile = fopen(path.data(), "rb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", path.data());
        return;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void)jpeg_read_header(&cinfo, TRUE);
    (void)jpeg_start_decompress(&cinfo);
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)(
        (j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    char* data = new char[ cinfo.image_height * row_stride ];
    char* data_iterator = data;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
        for (int i = 0; i < row_stride; ++i)
        {
            *(data_iterator++) = static_cast<char>(buffer[ 0 ][ i ]);
        }
    }

    image::metadata md;
    md._file_format = image::file_format::JPG;
    md._width = cinfo.output_width;
    md._height = cinfo.output_height;
    md._channel_count = cinfo.out_color_components;
    md._bits_per_pixel = 8;
    md._bytes_per_row = cinfo.output_width * cinfo.output_components;
    switch (cinfo.out_color_space)
    {
    case JCS_GRAYSCALE:
    {
        md._color_type = image::color_type::GRAYSCALE;
        break;
    }
    case JCS_RGB:
    {
        md._color_type = image::color_type::RGB;
        break;
    }
    case JCS_YCbCr:
    {
        md._color_type = image::color_type::YCbCr;
        break;
    }
    case JCS_CMYK:
    {
        md._color_type = image::color_type::CMYK;
        break;
    }
    case JCS_YCCK:
    {
        md._color_type = image::color_type::YCCK;
        break;
    }
    default:
    {
        md._color_type = image::color_type::UNSPECIFIED;
        break;
    }
    }

    _image = new image;
    _image->init(md);
    _image->set_data(data);

    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
}

image* asset_loader_JPG::get_image() { return _image; }
