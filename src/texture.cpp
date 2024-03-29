// TODO: investigate PBO and integrate
// https://www.songho.ca/opengl/gl_pbo.html

#include <string>

#include <glad/gl.h>

#include "texture.hpp"

#include "image.hpp"
#include "logging.hpp"

namespace
{
logger log() { return get_logger("texture"); }
} // namespace

texture::texture()
{
    glGenTextures(1, &_texture_id);
    _textures.push_back(this);
    log()->info("New texture created {}. Total number of textures {}",
                _texture_id,
                _textures.size());
}

texture::texture(texture&& other)
{
    _texture_id = other._texture_id;
    _width = other._width;
    _height = other._height;
    _format = other._format;
    other._texture_id = 0;
    _textures.push_back(this);
    log()->info("New texture created {}. Total number of textures {}",
                _texture_id,
                _textures.size());
}

texture& texture::operator=(texture&& other)
{
    _texture_id = other._texture_id;
    _width = other._width;
    _height = other._height;
    _format = other._format;
    other._texture_id = 0;
    return *this;
}

texture::~texture()
{
    int old_id = _texture_id;
    glDeleteTextures(1, &_texture_id);
    _textures.erase(std::find(_textures.begin(), _textures.end(), this));
    log()->info("Texture deleted {}. Total number of textures {}",
                old_id,
                _textures.size());
}

void texture::init(size_t width, size_t height, format texture_format)
{
    _width = width;
    _height = height;
    _format = texture_format;
    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 convert_to_gl_internal_format(texture_format),
                 _width,
                 _height,
                 0,
                 convert_to_gl_format(texture_format),
                 GL_UNSIGNED_BYTE,
                 nullptr);
    auto error = glGetError();
    if (error != GL_NO_ERROR)
    {
        return;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return;
}

glm::uvec2 texture::get_size() const { return { _width, _height }; }

size_t texture::get_width() const { return _width; }

size_t texture::get_height() const { return _height; }

size_t texture::get_channel_count() const
{
    switch (_format)
    {
    case format::GRAYSCALE:
    case format::DEPTH: return 1;
    case format::RGB: return 3;
    case format::RGBA: return 4;
    default: return 0;
    }
}

void texture::get_data(char* data_ptr)
{
    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_ptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void texture::set_data(const char* data_ptr)
{
    set_rect_data({ 0, 0 }, { _width, _height }, data_ptr);
}

void texture::set_rect_data(glm::vec<2, size_t> pos,
                            glm::vec<2, size_t> size,
                            const char* data_ptr)
{
    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    pos.x,
                    pos.y,
                    size.x,
                    size.y,
                    convert_to_gl_format(_format),
                    GL_UNSIGNED_BYTE,
                    data_ptr);
}

void texture::bind() const { static_bind(_texture_id); }

void texture::unbind() const { static_unbind(); }

void texture::set_active_texture(size_t index) const
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, _texture_id);
}

void texture::clone(const texture* other_texture)
{
    if (get_width() != other_texture->get_width() ||
        get_height() != other_texture->get_height() ||
        _format != other_texture->_format)
    {
        init(other_texture->get_width(),
             other_texture->get_height(),
             other_texture->_format);
    }

    glCopyImageSubData(other_texture->_texture_id,
                       GL_TEXTURE_2D,
                       0,
                       0,
                       0,
                       0,
                       _texture_id,
                       GL_TEXTURE_2D,
                       0,
                       0,
                       0,
                       0,
                       get_width(),
                       get_height(),
                       1);
}

unsigned texture::native_id() const { return _texture_id; }

texture texture::from_image(image* img)
{
    auto md = img->get_metadata();
    texture result;
    format tformat = format::UNSPECIFIED;
    switch (md._color_type)
    {
    case image::color_type::GRAYSCALE:
    {
        tformat = format::GRAYSCALE;
        break;
    }
    case image::color_type::RGB:
    {
        tformat = format::RGB;
        break;
    }
    case image::color_type::RGBA:
    {
        tformat = format::RGBA;
        break;
    }
    default:
    {
        break;
    }
    }
    result.init(img->get_width(), img->get_height(), tformat);
    result.set_data(img->get_data());
    return result;
}

void texture::static_bind(size_t id) { glBindTexture(GL_TEXTURE_2D, id); }

void texture::static_unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

int texture::convert_to_gl_internal_format(format f)
{
    switch (f)
    {
    case format::DEPTH: return GL_DEPTH_COMPONENT;
    case format::GRAYSCALE: return GL_RED;
    case format::RGB: return GL_RGB;
    case format::RGBA: return GL_RGBA;
    default: return GL_NONE;
    }
}

int texture::convert_to_gl_format(format f)
{
    switch (f)
    {
    case format::DEPTH: return GL_DEPTH_COMPONENT;
    case format::GRAYSCALE: return GL_RED;
    case format::RGB: return GL_RGB;
    case format::RGBA: return GL_RGBA;
    default: return GL_NONE;
    }
}

std::vector<texture*> texture::_textures;
