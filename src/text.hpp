#pragma once

#include <string>

#include "font.hpp"
#include "shader.hpp"

class text
{
public:
    struct color
    {
        float r;
        float g;
        float b;
    };

    struct position
    {
        float x;
        float y;
    };

public:
    text();

    void init();
    void set_font(font font);
    void set_text(std::string text);
    void set_shader(shader_program prog);
    void set_color(color text_color);
    void set_position(position text_position);
    void set_scale(float scale);
    void render() const;

private:
    font _font;
    shader_program _shader;
    color _color;
    position _position;
    float _scale;
    std::string _text;

    unsigned tvao = 0;
    unsigned tvbo = 0;
};