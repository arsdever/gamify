#include <glad/gl.h>

#include "graphics/framebuffer.hpp"

#include "common/logging.hpp"
#include "graphics/texture.hpp"

namespace
{
logger log() { return get_logger("framebuffer"); }
} // namespace

struct framebuffer::private_data
{
    glm::uvec2 _size { 0, 0 };
    unsigned _fbo { 0 };
    unsigned _copy_fbo { 0 };
    unsigned _sample_count { 1 };
    std::shared_ptr<graphics::texture> _color_texture {
        std::make_shared<graphics::texture>()
    };
    std::shared_ptr<graphics::texture> _depth_texture {
        std::make_shared<graphics::texture>()
    };
};

framebuffer::framebuffer() { _p = std::make_unique<private_data>(); }

framebuffer::~framebuffer() { destroy(); }

void framebuffer::initialize()
{
    if (_p->_fbo == 0)
    {
        glGenFramebuffers(1, &_p->_fbo);
    }

    bind();

    _p->_color_texture->init(
        _p->_size.x, _p->_size.y, graphics::texture::format::RGBA);
    _p->_depth_texture->init(
        _p->_size.x, _p->_size.y, graphics::texture::format::DEPTH);

    log()->info("Framebuffer {}:\n\ttextures::color {}\n\ttextures::depth {}",
                _p->_fbo,
                _p->_color_texture->native_id(),
                _p->_depth_texture->native_id());
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           _p->_sample_count > 1 ? GL_TEXTURE_2D_MULTISAMPLE
                                                 : GL_TEXTURE_2D,
                           _p->_color_texture->native_id(),
                           0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           _p->_sample_count > 1 ? GL_TEXTURE_2D_MULTISAMPLE
                                                 : GL_TEXTURE_2D,
                           _p->_depth_texture->native_id(),
                           0);
    auto fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fbo_status != GL_FRAMEBUFFER_COMPLETE)
    {
        log()->error(
            "Framebuffer {} is not complete! {}", _p->_fbo, fbo_status);
        unbind();
        return;
    }

    std::array<unsigned, 1> buffers { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(buffers.size(), buffers.data());
}

void framebuffer::destroy()
{
    glDeleteFramebuffers(1, &_p->_fbo);
    glDeleteFramebuffers(1, &_p->_copy_fbo);
    _p->_fbo = 0;
    _p->_copy_fbo = 0;
}

void framebuffer::set_samples(unsigned sample_count)
{
    _p->_sample_count = sample_count;
    _p->_color_texture->set_samples(sample_count);
    _p->_depth_texture->set_samples(sample_count);
}

std::shared_ptr<const graphics::texture> framebuffer::color_texture() const
{
    return _p->_color_texture;
}

std::shared_ptr<const graphics::texture> framebuffer::depth_texture() const
{
    return _p->_depth_texture;
}

void framebuffer::copy_texture(std::shared_ptr<graphics::texture> txt) const
{
    if (_p->_sample_count == 1)
    {
        txt->clone(color_texture().get());
        return;
    }

    txt->init(_p->_size.x, _p->_size.y);

    if (_p->_copy_fbo == 0)
    {
        glGenFramebuffers(1, &_p->_copy_fbo);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _p->_copy_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           txt->native_id(),
                           0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _p->_fbo);
    glBlitFramebuffer(0,
                      0,
                      _p->_size.x,
                      _p->_size.y,
                      0,
                      0,
                      _p->_size.x,
                      _p->_size.y,
                      GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::blit(unsigned framebuffer_id) const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _p->_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
    glBlitFramebuffer(0,
                      0,
                      _p->_size.x,
                      _p->_size.y,
                      0,
                      0,
                      _p->_size.x,
                      _p->_size.y,
                      GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
}

void framebuffer::resize(glm::uvec2 size)
{
    if (_p->_size != size)
    {
        _p->_size = size;
        _p->_color_texture->init(
            size.x, size.y, graphics::texture::format::RGBA);
        _p->_depth_texture->init(
            size.x, size.y, graphics::texture::format::DEPTH);
    }
}

void framebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, _p->_fbo); }

void framebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
