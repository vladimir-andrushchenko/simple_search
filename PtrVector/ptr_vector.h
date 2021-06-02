#pragma once


#include <algorithm>
#include <cassert>
#include <vector>

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

public:
    // Возвращает ссылку на вектор указателей
    std::vector<T*>& GetItems() noexcept {
        return items_;
    }

    // Возвращает константную ссылку на вектор указателей
    std::vector<T*> const& GetItems() const noexcept {
        return items_;
    }
    
    typename std::vector<T*>::const_iterator begin() {
        return items_.begin();
    }
    
    typename std::vector<T*>::const_iterator end() {
        return items_.end();
    }
    
    auto size() const {
        return items_.size();
    }
    
    auto at(size_t i) const {
        return items_.at(i);
    }
    
    void clear() {
        items_.clear();
    }
    
    void push_back(T* element) {
        items_.push_back(element);
    }

private:
    std::vector<T*> items_;
};
