#include "core/timer.hpp"

#include "common/logging.hpp"

namespace core
{
timer::timer() { }

timer::~timer()
{
    cancel();
    wait();
}

void timer::start(std::chrono::duration<double> delay)
{
    _delay = delay;
    _cancelled.store(false);
    _thread = std::thread([ this ]()
    {
        _is_running.store(true);
        if (run_and_wait(_delay))
        {
            tick();
            if (_iteration)
            {
                while (run_and_wait(_iteration.value()))
                {
                    tick();
                }
            }
        }
        _is_running.store(false);
    });
}

void timer::set_iteration(std::chrono::duration<double> iteration)
{
    _iteration = iteration;
}

void timer::set_not_repeating() { _iteration = std::nullopt; }

void timer::wait()
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void timer::cancel()
{
    _cancelled.store(true);
    _timer_stopper.notify_all();
}

void timer::reset()
{
    if (is_running())
        return;

    _cancelled.store(false);
}

bool timer::is_running() { return _is_running.load(); }

bool timer::is_cancelled() { return _cancelled.load(); }

void timer::single_shot(std::chrono::duration<double> delay,
                        std::function<void()> callback)
{
    timer t;
    t.tick += callback;
    t.start(delay);
    t.wait();
}

bool timer::run_and_wait(std::chrono::duration<double> sleep_time)
{
    _start_time = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lock(_mutex);
    auto status = _timer_stopper.wait_for(lock,
                                          sleep_time,
                                          [ this, sleep_time ]()
    {
        return is_cancelled() ||
               std::chrono::steady_clock::now() - _start_time >= sleep_time;
    });
    return !_cancelled.load();
}
} // namespace core
