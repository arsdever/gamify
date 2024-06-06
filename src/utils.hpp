#pragma once

struct string_hash
{
    using is_transparent = void; // enables heterogenous lookup

    std::size_t operator()(const char* char_string) const
    {
        std::hash<std::string_view> hasher;
        return hasher(char_string);
    }
    std::size_t operator()(std::string_view string_view) const
    {
        std::hash<std::string_view> hasher;
        return hasher(string_view);
    }
};

// geometry utils

template <typename T>
bool is_in_range(T rb, T re, T v)
{
    return v >= rb && v <= re;
}

template <typename T>
bool rect_contains(glm::vec<2, T, glm::defaultp> top_left,
                   glm::vec<2, T, glm::defaultp> bottom_right,
                   glm::vec<2, T, glm::defaultp> pos)
{
    glm::vec<2, T, glm::defaultp> tl = top_left;
    glm::vec<2, T, glm::defaultp> br = bottom_right;
    if (top_left.x > bottom_right.x)
    {
        tl.x = bottom_right.x;
        br.x = top_left.x;
    }
    if (top_left.y > bottom_right.y)
    {
        tl.y = bottom_right.y;
        br.y = top_left.y;
    }

    return is_in_range(tl.x, br.x, pos.x) && is_in_range(tl.y, br.y, pos.y);
}

template <typename T>
class singleton
{
public:
    static std::shared_ptr<T> get() { return _instance; }

private:
    using singleton_t = singleton<T>;

    singleton() = default;
    singleton(const singleton&) = delete;
    singleton& operator=(const singleton&) = delete;
    singleton(singleton&&) = delete;
    singleton& operator=(singleton&&) = delete;
    ~singleton() = default;

    friend T;

    static std::shared_ptr<T> _instance;
};

template <typename T>
std::shared_ptr<T> singleton<T>::_instance = std::shared_ptr<T>(new T());

template <typename R, typename... ARGS>
R gl_convert(ARGS&&... args);
