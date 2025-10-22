// lv_cpp_utils.hpp
#pragma once
#include "lvgl.h"
#include <functional>

namespace lvgl
{
    inline void post(std::function<void()> fn)
    {
        auto *heap_fn = new std::function<void()>(std::move(fn));
        
        lv_async_call([](void *param)
        {
            auto *fn = static_cast<std::function<void()>*>(param);
            (*fn)();
            delete fn;
        }, 
        heap_fn);
    }
} // namespace lvgl