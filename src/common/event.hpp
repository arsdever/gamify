#pragma once

template <typename T>
class event
{
public:
    class connection
    {
        using is_transparent = void;

    public:
        connection(const connection& other)
            : _id(other._id)
            , _event(other._event)
            , _func(other._func)
            , _auto_drop(other._auto_drop)
        {
        }

        connection(connection&& other)
            : _id(other._id)
            , _event(other._event)
            , _func(std::move(other._func))
            , _auto_drop(other._auto_drop)
        {
            other._id = 0;
            other._auto_drop = false;
        }

        connection& operator=(const connection&) = delete;

        connection& operator=(connection&& other)
        {
            if (this != &other)
            {
                _id = other._id;
                _event = other._event;
                _func = std::move(other._func);
                _auto_drop = other._auto_drop;
                other._id = 0;
            }
        }

        ~connection()
        {
            if (_auto_drop)
            {
                drop();
            }
        }

        void drop() { _event._listeners.erase(*this); }

        void set_autodrop(bool flag) { _auto_drop = flag; }

        bool operator<(const connection& other) const
        {
            return _id < other._id;
        }

        bool operator<(size_t id) const { return _id < id; }

    private:
        connection() = default;

        connection(size_t id, event& e, std::function<T> func)
            : _id(id)
            , _event(e)
            , _func(func)
        {
        }

        friend class event<T>;

        template <typename... ARGS>
        void operator()(ARGS&&... args) const
        {
            _func(std::forward<ARGS>(args)...);
        }

    private:
        static size_t _id_counter;
        size_t _id;
        event<T>& _event;
        std::function<T> _func;
        bool _auto_drop { false };
    };

    event() = default;

    template <typename... ARGS>
    void operator()(ARGS&&... args)
    {
        for (auto& listener : _listeners)
        {
            listener(std::forward<ARGS>(args)...);
        }
    }

    template <typename F>
    connection operator+=(F&& listener)
    {
        auto [ it, _ ] = _listeners.insert(connection {
            ++connection::_id_counter, *this, std::forward<F>(listener) });
        (void)_;
        return *it;
    }

    void reset() { _listeners.clear(); }

    bool has_listeners() { return !_listeners.empty(); }

private:
    std::set<connection> _listeners;
};

template <typename T>
size_t event<T>::connection::_id_counter = 0;
