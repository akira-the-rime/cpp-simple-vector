#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

#include "array_ptr.h"

using namespace std::string_literals;

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(std::size_t size) : pointer(size), size_(size), capacity_(size) {
        AssignElements(*this);
    }

    SimpleVector(size_t size, const Type& value) : pointer(size), size_(size), capacity_(size) {
        std::fill_n(pointer.Get(), capacity_, value);
    }

    SimpleVector(std::initializer_list<Type> init) : pointer(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(std::make_move_iterator(init.begin()), std::make_move_iterator(init.end()), pointer.Get());
    }

    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> temp(other.capacity_);
        std::copy(this->begin(), this->end(), temp.begin());
        temp.size_ = other.size_;
        this->swap(temp);
    }

    SimpleVector(SimpleVector&& other) noexcept : size_(other.size_), capacity_(other.capacity_) {
        this->pointer = std::move(other.pointer);
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> temp(rhs);
            this->swap(temp);
        }
        return *this;
    }

    void PushBack(Type item) {
        if (size_ == capacity_) {
            std::size_t new_capacity = capacity_ ? capacity_ * 2 : 1;
            SimpleVector<Type> temp(new_capacity);
            for (std::size_t i = 0; i < this->size_; ++i) {
                temp[i] = std::move(this->At(i));
            }
            temp.size_ = this->size_;
            this->swap(temp);
        }
        this->Add(size_) = std::move(item);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, Type value) {
        assert(pos >= this->cbegin() && pos <= this->cend());
        std::size_t to_insert = pos - this->begin();
        if (size_ == capacity_) {
            std::size_t new_capacity = capacity_ ? capacity_ * 2 : 1;
            SimpleVector<Type> temp(new_capacity);
            std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->end()), temp.begin());
            temp.size_ = this->size_;
            this->swap(temp);
        }
        SimpleVector<Type> temp(this->capacity_ + 1);
        Iterator it = std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->begin() + to_insert), temp.begin());
        *(it++) = std::move(value);
        std::copy(std::make_move_iterator(this->begin() + to_insert), std::make_move_iterator(this->end()), it);
        this->pointer = std::move(temp.pointer);
        ++size_;
        return this->begin() + to_insert;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector<Type> temp(new_capacity);
            std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->end()), temp.begin());
            temp.size_ = this->size_;
            this->swap(temp);
        }
    }

    void PopBack() noexcept {
        if (!this->IsEmpty()) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        std::size_t shift = pos - this->begin();
        if (size_) {
            SimpleVector<Type> temp(this->capacity_ - 1);
            Iterator it = std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->begin() + shift), temp.begin());
            std::copy(std::make_move_iterator(this->begin() + shift + 1), std::make_move_iterator(this->end()), it);
            this->pointer = std::move(temp.pointer);
            --size_;
            return size_ ? this->begin() + shift : nullptr;
        }
        return nullptr;
    }

    void swap(SimpleVector& other) noexcept {
        this->pointer.swap(other.pointer);
        std::swap(this->size_, other.size_);
        std::swap(this->capacity_, other.capacity_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < this->size_);
        return pointer[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < this->size_);
        return pointer[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Out of range."s);
        }
        return pointer[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Out of range."s);
        }
        return pointer[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (size_ >= new_size) {
            size_ = new_size;
        }
        else {
            std::size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            AssignElements(temp, new_capacity);
            for (std::size_t i = 0; i < this->size_; ++i) {
                temp[i] = std::move(this->pointer[i]);
            }
            this->pointer.swap(temp);
            this->size_ = new_size;
            this->capacity_ = new_capacity;
        }
    }

    Iterator begin() noexcept {
        return pointer.Get();
    }

    Iterator end() noexcept {
        return pointer.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return pointer.Get();
    }

    ConstIterator end() const noexcept {
        return pointer.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return pointer.Get();
    }

    ConstIterator cend() const noexcept {
        return pointer.Get() + size_;
    }

private:

    Type& Add(size_t index) {
        if (index > size_) {
            throw std::out_of_range("Out of range."s);
        }
        return pointer[index];
    }

    void AssignElements(ArrayPtr<Type>& entity, std::size_t new_capacity) noexcept {
        for (std::size_t i = 0; i < new_capacity; ++i) {
            entity[i] = std::move(Type{});
        }
    }

    void AssignElements(SimpleVector<Type>& entity) noexcept {
        for (std::size_t i = 0; i < entity.size_; ++i) {
            entity[i] = std::move(Type{});
        }
    }

    ArrayPtr<Type> pointer;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

template <class Type>
SimpleVector<Type> Reserve(std::size_t size) {
    SimpleVector<Type> created(size);
    created.Clear();
    return created;
}

template <class Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <class Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <class Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <class Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}