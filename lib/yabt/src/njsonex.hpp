#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <vector>
#include <memory_resource>
#include "esp_log.h"

//struct FixedPoolBase
//{
//    uint8_t *buf;
//    size_t capacity;
//    size_t offset;
//};
//
//template <size_t N>
//struct FixedPool : FixedPoolBase
//{
//    uint8_t storage[N];
//
//    FixedPool()
//    {
//        buf = storage;
//        capacity = N;
//        offset = 0;
//    }
//};
//
//template <size_t N>
//struct FixedPool111
//{
//    uint8_t buf[N];
//    size_t offset = 0;
//    size_t capacity = N;
//};
//
//template <class T>
//class FixedAllocator
//{
//public:
//    using value_type = T;
//    FixedPoolBase *pool = nullptr;
//
//    FixedAllocator() = default;
//
//    FixedAllocator(FixedPoolBase *p) : pool(p) {}
//
//    template <class U>
//    FixedAllocator(const FixedAllocator<U> &other)
//        : pool(other.pool)
//    {
//    }
//
//    T *allocate(size_t n)
//    {
//        size_t bytes = n * sizeof(T);
//
//        size_t aligned =
//            (pool->offset + alignof(T) - 1) & ~(alignof(T) - 1);
//
//        if (aligned + bytes > pool->capacity)
//            ESP_LOGE("FixedAllocator",
//                     "Out of memory: need %u bytes, have %u, capacity %u",
//                     bytes, pool->capacity - pool->offset, pool->capacity);
//
//        T *ptr = reinterpret_cast<T *>(pool->buf + aligned);
//        pool->offset = aligned + bytes;
//        return ptr;
//    }
//
//    void deallocate(T *, size_t) {}
//
//    template <class U>
//    struct rebind
//    {
//        using other = FixedAllocator<U>;
//    };
//};
//
//using PoolVector = std::vector<uint8_t, FixedAllocator<uint8_t>>;




// 1. Создаем пользовательский класс-обертку для логирования
class LoggingMemoryResource : public std::pmr::memory_resource
{
public:
    // Конструктор принимает указатель на реальный ресурс (в нашем случае - кучу)
    explicit LoggingMemoryResource(std::pmr::memory_resource *upstream)
        : upstream_resource_(upstream) {}

    bool used_fallback() const { return used_fallback_; }

private:
    std::pmr::memory_resource *upstream_resource_;
    bool used_fallback_ = false;

    // Переопределяем функции выделения/освобождения
    void *do_allocate(size_t bytes, size_t alignment) override
    {
        // Устанавливаем флаг, что мы были вынуждены выделить память здесь (в куче)
        used_fallback_ = true;
        ESP_LOGE("LoggingMemoryResource", "[LOG] не хватило места на стеке! Выделяем %u байт(а) в куче.", bytes);
        return upstream_resource_->allocate(bytes, alignment);
    }

    void do_deallocate(void *ptr, size_t bytes, size_t alignment) override
    {
        ESP_LOGE("LoggingMemoryResource", "[LOG] Освобождаем %u байт(а) из кучи.", bytes);
        upstream_resource_->deallocate(ptr, bytes, alignment);
    }

    // Обязательная функция сравнения ресурсов
    bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override
    {
        // Сравниваемся с самим собой или с нижележащим ресурсом
        return &other == this || upstream_resource_->is_equal(other);
    }
};


using PoolVectorPmr = std::pmr::vector<std::uint8_t>;