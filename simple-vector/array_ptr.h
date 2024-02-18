#pragma once

#include <cstdlib>
#include <stdexcept>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        if (size > 0) {
            try {
                raw_ptr_ = new Type[size];
            }
            catch (const std::bad_alloc&) {
                delete[] raw_ptr_;
                throw std::bad_alloc();
            }
        }
        else {
            raw_ptr_ = nullptr;
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept : raw_ptr_(raw_ptr) { }

    ArrayPtr(const ArrayPtr&) = delete;

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& other){
        this->raw_ptr_ = other.Release();
        return *this;
    }

    [[nodiscard]] Type* Release() noexcept {
        Type* temp = raw_ptr_;
        raw_ptr_ = nullptr;
        return temp;
    }

    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const {
        return raw_ptr_ ? true : false;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        Type* temp = this->raw_ptr_;
        this->raw_ptr_ = other.raw_ptr_;
        other.raw_ptr_ = temp;
    }
private:
    Type* raw_ptr_ = nullptr;
};