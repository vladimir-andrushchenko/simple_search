#pragma once


#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;

template <typename T>
class PtrVector {
public:
    PtrVector() = default;

    // Создаёт вектор указателей на копии объектов из other
    PtrVector(const PtrVector& other) {
        items_.clear();
        
        for (const T* item : other.GetItems()) {
            if (item == nullptr) {
                items_.push_back(nullptr);
                continue;
            }
            
            items_.push_back(new T(*item));
        }
    }

    // Деструктор удаляет объекты в куче, на которые ссылаются указатели,
    // в векторе items_
    ~PtrVector() {
        for (auto item : items_) {
            delete item;
        }
    }

    // Возвращает ссылку на вектор указателей
    vector<T*>& GetItems() noexcept {
        return items_;
    }

    // Возвращает константную ссылку на вектор указателей
    vector<T*> const& GetItems() const noexcept {
        return items_;
    }

private:
    vector<T*> items_;
};
