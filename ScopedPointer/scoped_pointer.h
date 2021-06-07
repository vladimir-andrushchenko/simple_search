#pragma once

#include <stdexcept>
#include <string>

using namespace std::literals;

template <typename T>
class ScopedPtr {
public:
    ScopedPtr() = default;

    explicit ScopedPtr(T* raw_ptr) noexcept {
        ptr_ = raw_ptr;
    }

    ScopedPtr(const ScopedPtr&) = delete;

    ~ScopedPtr() {
        delete ptr_;
        ptr_ = nullptr;
    }

public:
    T* GetRawPtr() const noexcept {
        return ptr_;
    }

    T* Release() noexcept {
        auto temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }
    
        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }

        T& operator*() const {
            if (ptr_ == nullptr) {
                throw std::logic_error("points to nullptr"s);
            }
            
            return *ptr_;
        }

        T* operator->() const {
            if (ptr_ == nullptr) {
                throw std::logic_error("points to nullptr"s);
            }
            
            return ptr_;
        }

private:
    T* ptr_ = nullptr;
};
