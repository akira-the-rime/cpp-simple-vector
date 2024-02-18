#include <cassert>
#include <iostream>
#include <numeric>

#include "simple_vector.h"

void TestTemporaryObjConstructor();
void TestTemporaryObjOperator();
void TestNamedMoveConstructor();
void TestNamedMoveOperator();
void TestNoncopiableMoveConstructor();
void TestNoncopiablePushBack();
void TestNoncopiableInsert();
void TestNoncopiableErase();

int main() {
    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
    return 0;
}

class X {
public:
    X() : X(5) { }

    X(size_t num) : x_(num) { }

    X(const X& other) = delete;
    X& operator=(const X& other) = delete;

    X(X&& other) noexcept {
        x_ = std::exchange(other.x_, 0);
    }

    X& operator=(X&& other) noexcept {
        x_ = std::exchange(other.x_, 0);
        return *this;
    }

    size_t GetX() const {
        return x_;
    }
private:
    size_t x_;
};

SimpleVector<int> GenerateVector(size_t size) {
    SimpleVector<int> v(size);
    std::iota(v.begin(), v.end(), 1);
    return v;
}

void TestTemporaryObjConstructor() {
    const size_t size = 1000000;
    std::cout << "Test with temporary object, copy elision"s << std::endl;
    SimpleVector<int> moved_vector(GenerateVector(size));
    assert(moved_vector.GetSize() == size);
    std::cout << "Done!"s << std::endl << std::endl;
}

void TestTemporaryObjOperator() {
    const size_t size = 1000000;
    std::cout << "Test with temporary object, operator="s << std::endl;
    SimpleVector<int> moved_vector;
    assert(moved_vector.GetSize() == 0);
    moved_vector = GenerateVector(size);
    assert(moved_vector.GetSize() == size);
    std::cout << "Done!"s << std::endl << std::endl;
}

void TestNamedMoveConstructor() {
    const size_t size = 1000000;
    std::cout << "Test with named object, move constructor"s << std::endl;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    assert(vector_to_move.GetSize() == size);

    SimpleVector<int> moved_vector(std::move(vector_to_move));
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);
    std::cout << "Done!"s << std::endl << std::endl;
}

void TestNamedMoveOperator() {
    const size_t size = 1000000;
    std::cout << "Test with named object, operator="s << std::endl;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    assert(vector_to_move.GetSize() == size);

    SimpleVector<int> moved_vector = std::move(vector_to_move);
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);
    std::cout << "Done!"s << std::endl << std::endl;
}

void TestNoncopiableMoveConstructor() {
    const size_t size = 5;
    std::cout << "Test noncopiable object, move constructor"s << std::endl;
    SimpleVector<X> vector_to_move;
    for (size_t i = 0; i < size; ++i) {
        vector_to_move.PushBack(X(i));
    }

    SimpleVector<X> moved_vector = std::move(vector_to_move);
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);

    for (size_t i = 0; i < size; ++i) {
        assert(moved_vector[i].GetX() == i);
    }
    std::cout << "Done!"s << std::endl << std::endl;
}

void TestNoncopiablePushBack() {
    const size_t size = 5;
    std::cout << "Test noncopiable push back"s << std::endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    assert(v.GetSize() == size);

    for (size_t i = 0; i < size; ++i) {
        assert(v[i].GetX() == i);
    }
    std::cout << "Done!"s << std::endl << std::endl;
}

void TestNoncopiableInsert() {
    const size_t size = 5;
    std::cout << "Test noncopiable insert"s << std::endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    // в начало
    v.Insert(v.begin(), X(size + 1));
    assert(v.GetSize() == size + 1);
    assert(v.begin()->GetX() == size + 1);
    // в конец
    v.Insert(v.end(), X(size + 2));
    assert(v.GetSize() == size + 2);
    assert((v.end() - 1)->GetX() == size + 2);
    // в середину
    v.Insert(v.begin() + 3, X(size + 3));
    assert(v.GetSize() == size + 3);
    assert((v.begin() + 3)->GetX() == size + 3);
    std::cout << "Done!"s << std::endl << std::endl;
}

void TestNoncopiableErase() {
    const size_t size = 3;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    auto it = v.Erase(v.begin());
    assert(it->GetX() == 1);
    std::cout << "Done!"s << std::endl << std::endl;
}