#pragma once

class image
{
public:
    image();

    void init(size_t width, size_t height);
    void set_data(char* data);
    const char* get_data() ;

    size_t width() const;
    size_t height() const;

private:
    size_t _width;
    size_t _height;
    char* _data;
};