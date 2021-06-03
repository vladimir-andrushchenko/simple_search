#pragma once

#include <iterator>
#include <cstddef>

using namespace std;

template <typename Type>
class SingleLinkedList {
public:
    ~SingleLinkedList() {
        Clear();
    }
    
public:
    // Возвращает количество элементов в списке за время O(1)
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }
    
    // Сообщает, пустой ли список за время O(1)
    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0;
    }
    
    // Вставляет элемент value в начало списка за время O(1)
    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }
    
    // Очищает список за время O(N)
    void Clear() noexcept {
        while (head_.next_node) {
            Node* current_node = head_.next_node;
            head_.next_node = current_node->next_node;
            delete current_node;
            
            --size_;
        }
    }
    
private:
    struct Node {
        Node() = default;
        
        Node(const Type& val, Node* next)
        : value(val)
        , next_node(next) {
        }
        
        Type value;
        Node* next_node = nullptr;
    };
    
private:
    Node head_{};
    size_t size_ = 0;
};

