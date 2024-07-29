#include <stack>

#include <freetype/freetype.h>

#include "graphics/font.hpp"

#include "common/logging.hpp"
#include "glad/gl.h"
#include "graphics/texture.hpp"

namespace
{
static logger log() { return get_logger("font"); }
} // namespace

font::font() = default;

void font::load(std::string path, float size)
{
    _font_file_path = std::move(path);
    _font_size = size;
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        log()->error("FREETYPE: Could not init FreeType Library");
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, _font_file_path.c_str(), 0, &face))
    {
        log()->error("FREETYPE: Failed to load font");
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, _font_size);
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        log()->error("FREETYTPE: Failed to load Glyph");
        return;
    }

    font_node node {
        glm::uvec2 { 0 }, glm::uvec2 { INT_MAX }, -1, nullptr, nullptr
    };

    _atlas = std::make_shared<texture>();
    _atlas->init(1024, 1024, texture::format::GRAYSCALE);
    _atlas->set_wrapping_mode(
        true, true, texture::wrapping_mode::clamp_to_edge);
    _atlas->set_sampling_mode_mag(texture::sampling_mode::linear);
    _atlas->set_sampling_mode_min(texture::sampling_mode::linear);
    glPixelStorei(GL_UNPACK_ALIGNMENT,
                  1); // disable byte-alignment restriction

    for (unsigned int c = 0; c < 256; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            log()->error("FREETYTPE: Failed to load Glyph");
            continue;
        }

        // TODO: use our texture class when it supports custom color types
        // generate texture
        // now store character for later use

        auto fn = node.insert(
            c,
            glm::uvec2 { face->glyph->bitmap.width, face->glyph->bitmap.rows });

        if (!fn)
        {
            log()->error("FREETYTPE: Failed to insert Glyph {}({})",
                         static_cast<int>(c),
                         static_cast<char>(c));
            continue;
        }

        character ch = { fn->_origin,
                         { face->glyph->bitmap.width,
                           face->glyph->bitmap.rows },
                         { face->glyph->bitmap_left, face->glyph->bitmap_top },
                         static_cast<float>(face->glyph->advance.x) };
        _character_map.insert(std::pair<char, character>(c, ch));

        _atlas->set_rect_data(
            fn->_origin,
            fn->_size,
            reinterpret_cast<const char*>(face->glyph->bitmap.buffer));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    log()->info("Done packing");
}

glm::vec2 font::get_text_size(std::string_view text) const
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        log()->error("FREETYPE: Could not init FreeType Library");
        return {};
    }

    FT_Face face;
    if (FT_New_Face(ft, _font_file_path.c_str(), 0, &face))
    {
        log()->error("FREETYPE: Failed to load font");
        return {};
    }

    FT_Set_Pixel_Sizes(face, 0, _font_size);
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        log()->error("FREETYTPE: Failed to load Glyph");
        return {};
    }

    glm::vec2 pen { 0, 0 };
    glm::vec2 size { 0 };

    size.y = face->size->metrics.height;

    for (auto ch : text)
    {
        // load character glyph
        if (FT_Load_Char(face, ch, FT_LOAD_NO_BITMAP))
        {
            log()->error("FREETYTPE: Failed to load Glyph");
            continue;
        }

        size.x += face->glyph->advance.x;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return size / 64.0f;
}

texture& font::atlas() { return *_atlas; }

const texture& font::atlas() const { return *_atlas; }

font::font_node* font::font_node::insert(int ch, const glm::uvec2& font_size)
{
    if (_id > 0)
    {
        return nullptr;
    }

    if (_l != nullptr && _r != nullptr)
    {
        // check if the rectangle fits
        if (font_node* result = _l->insert(ch, font_size); result)
        {
            return result;
        }

        return _r->insert(ch, font_size);
    }

    // This is an unfilled leaf - let's see if we can fill it
    glm::uvec2 real_size(_size.x, _size.y);

    // If we're along a boundary, calculate the actual size
    if (_origin.x + _size.x == INT_MAX)
    {
        real_size.x = texture_size.x - _origin.x;
    }

    if (_origin.y + _size.y == INT_MAX)
    {
        real_size.y = texture_size.y - _origin.y;
    }

    if (_size.x == font_size.x && _size.y == font_size.y)
    {
        // Perfect size - just pack into this node
        _id = ch;
        return this;
    }
    else if (real_size.x < font_size.x || real_size.y < font_size.y)
    {
        // Not big enough
        return nullptr;
    }
    else
    {
        // Large enough - split until we get a perfect fit
        font_node* left { nullptr };
        font_node* right { nullptr };

        // Determine how much space we'll have left if we split each way
        int remain_x = real_size.x - font_size.x;
        int remain_y = real_size.y - font_size.y;

        // Split the way that will leave the most room
        bool verticalSplit = remain_x < remain_y;
        if (remain_x == 0 && remain_y == 0)
        {
            // Edge case - we are are going to hit the border of
            // the texture atlas perfectly, split at the border instead
            if (_size.x > _size.y)
            {
                verticalSplit = false;
            }
            else
            {
                verticalSplit = true;
            }
        }

        if (verticalSplit)
        {
            // Split vertically (left is top)
            left = new font_node(_origin, glm::uvec2(_size.x, font_size.y));
            right =
                new font_node(glm::uvec2(_origin.x, _origin.y + font_size.y),
                              glm::uvec2(_size.x, _size.y - font_size.y));
        }
        else
        {
            // Split horizontally
            left = new font_node(_origin, glm::uvec2(font_size.x, _size.y));
            right =
                new font_node(glm::uvec2(_origin.x + font_size.x, _origin.y),
                              glm::uvec2(_size.x - font_size.x, _size.y));
        }

        _l = std::unique_ptr<font_node>(left);
        _r = std::unique_ptr<font_node>(right);
        return _l->insert(ch, font_size);
    }
}

glm::uvec2 font::font_node::texture_size { 1024, 1024 };

const font::character& font::operator[](unsigned int ch) const
{
    return _character_map.at(ch);
}
