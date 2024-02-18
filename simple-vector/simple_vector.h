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
        for (std::size_t i = 0; i < size_; ++i) {
            ShoveElement(this->At(i), Type{});
        }
    }

    SimpleVector(size_t size, const Type& value) : pointer(size), size_(size), capacity_(size) {
        std::fill_n(pointer.Get(), capacity_, value);
    }

    SimpleVector(std::initializer_list<Type> init) : pointer(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(std::make_move_iterator(init.begin()), std::make_move_iterator(init.end()), pointer.Get());
    }

    // Этот конструктор нужен для того, чтобы инициализировать объект с помощью функции Resize().
    // Ее определение есть после определения этого класса.
    // Эта функция была одной из задач спринта.
    explicit SimpleVector(const std::pair<std::size_t, std::size_t>& data) : pointer(data.first), size_(data.first), capacity_(data.second) { }

    SimpleVector(const SimpleVector& other) {
        try {
            SimpleVector<Type> temp(other.capacity_);
            std::copy(this->begin(), this->end(), temp.begin());
            temp.size_ = other.size_;
            this->swap(temp);
        }
        catch (const std::bad_alloc&) {
            throw std::bad_alloc();
        }
    }

    SimpleVector(SimpleVector&& other) noexcept : size_(other.size_), capacity_(other.capacity_) {
        this->pointer = std::move(other.pointer);
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            try {
                SimpleVector<Type> temp(rhs);
                this->swap(temp);
            }
            catch (const std::bad_alloc&) {
                throw std::bad_alloc();
            }
        }
        return *this;
    }

    void PushBack(Type item) {
        if (size_ == capacity_) {
            try {
                std::size_t new_capacity = capacity_ ? capacity_ * 2 : 1;
                SimpleVector<Type> temp(new_capacity);
                for (std::size_t i = 0; i < this->size_; ++i) {
                    temp[i] = std::move(this->At(i));
                }
                temp.size_ = this->size_;
                this->swap(temp);
            }
            catch (const std::bad_alloc&) {
                throw std::bad_alloc();
            }
        }
        this->Add(size_) = std::move(item);
        ++size_;
    }

    // Перебрасываю исключение std::bad_alloc() в main, для того чтобы пользователь класса мог его обработать там.
    // 
    // Насчет двух версий Insert и PushBack в теории было написано, что универсальный вариант - это принять параметр по значению.
    // Если передаваемый аргумент временный, то он переместится в параметр метода, а если нет то скопируется. 
    // Далее, в методе, функция move будет работать только в том случае, если объект перемещаемый. 
    // Иначе будет происходить копирование, а функция move будет игнорироваться.
    Iterator Insert(ConstIterator pos, Type value) {
        assert(pos >= this->cbegin() || pos <= this->cend());
        std::size_t to_insert = pos - this->begin();
        if (size_ == capacity_) {
            try {
                std::size_t new_capacity = capacity_ ? capacity_ * 2 : 1;
                SimpleVector<Type> temp(new_capacity);
                std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->end()), temp.begin());
                temp.size_ = this->size_;
                this->swap(temp);
            }
            catch (const std::bad_alloc&) {
                throw std::bad_alloc();
            }
        }
        try {
            SimpleVector<Type> temp(this->capacity_ + 1);
            Iterator it = std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->begin() + to_insert), temp.begin());
            *(it++) = std::move(value);
            std::copy(std::make_move_iterator(this->begin() + to_insert), std::make_move_iterator(this->end()), it);
            this->pointer = std::move(temp.pointer);
            ++size_;
        }
        catch (const std::bad_alloc&) {
            throw std::bad_alloc();
        }
        return this->begin() + to_insert;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            try {
                SimpleVector<Type> temp(new_capacity);
                std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->end()), temp.begin());
                temp.size_ = this->size_;
                this->swap(temp);
            }
            catch (const std::bad_alloc&) {
                throw std::bad_alloc();
            }
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
            try {
                SimpleVector<Type> temp(this->capacity_ - 1);
                Iterator it = std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->begin() + shift), temp.begin());
                std::copy(std::make_move_iterator(this->begin() + shift + 1), std::make_move_iterator(this->end()), it);
                this->pointer = std::move(temp.pointer);
                --size_;
                return size_ ? this->begin() + shift : nullptr;
            }
            catch (std::bad_alloc&) {
                throw std::bad_alloc();
            }
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

    // В теории было написано, что тернарный оператор предпочтительнее, так как ветвление if заставляет замедлиться выполнение программы.
    bool IsEmpty() const noexcept {
        return (size_ != 0) ? false : true;
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
            try {
                std::size_t new_capacity = std::max(new_size, capacity_ * 2);
                ArrayPtr<Type> temp(new_capacity);
                for (std::size_t i = 0; i < new_capacity; ++i) {
                    ShoveElement(temp[i], Type{});
                }
                for (std::size_t i = 0; i < this->size_; ++i) {
                    temp[i] = std::move(this->pointer[i]);
                }
                this->pointer.swap(temp);
                this->size_ = new_size;
                this->capacity_ = new_capacity;
            }
            catch (const std::bad_alloc&) {
                throw std::bad_alloc();
            }
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

    void ShoveElement(Type& entity, Type&& element) noexcept {
        entity = std::move(element);
    }

    ArrayPtr<Type> pointer;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

std::pair<std::size_t, std::size_t> Reserve(std::size_t size) {
    return { 0, size };
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

// Lexicographical_compare() возвращает true, если "меньше" и false, если "больше или равно".
// (lhs < rhs) вернет истину только в том случае, если (lhs < rhs).
// Это нужно дополнить проверкой на равенство lhs и rhs, чтобы возвращаемое bool-значение соотв. оператору <=. 
template <class Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs) || (lhs == rhs);
}

// Тоже самое, что и с <=, только надо удостовериться, что lhs и rhs не равны.
// Так как !(lhs < rhs) вернет истину в случае, если lhs >= rhs.
template <class Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs) && (lhs != rhs);
}

template <class Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}