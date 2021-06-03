#pragma once

#include <iterator>
#include <cstddef>

using namespace std;

template <typename Type>
class SingleLinkedList {
public:
    template <typename ValueType>
    class BasicIterator;
    
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
    void PushFront(const Type& value);
    
    // Очищает список за время O(N)
    void Clear() noexcept;
    
public:
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;

    [[nodiscard]] Iterator begin() noexcept {
        auto before_begin_copy = before_begin_;
        return ++before_begin_copy;
    }
    
    [[nodiscard]] Iterator end() noexcept {
        return end_;
    }
    
    [[nodiscard]] ConstIterator begin() const noexcept {
        auto before_begin_copy = before_begin_;
        return ++before_begin_copy;
    }
    
    [[nodiscard]] ConstIterator end() const noexcept {
        return end_;
    }
    
    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return begin();
    }
    
    [[nodiscard]] ConstIterator cend() const noexcept {
        return end();
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
    
    Iterator before_begin_{&head_};
    Iterator end_{head_.next_node};
};

template <typename Type>
void SingleLinkedList<Type>::PushFront(const Type& value) {
    head_.next_node = new Node(value, head_.next_node);
    ++size_;
}

template <typename Type>
void SingleLinkedList<Type>::Clear() noexcept {
    while (head_.next_node) {
        Node* current_node = head_.next_node;
        head_.next_node = current_node->next_node;
        delete current_node;
        
        --size_;
    }
}

template <typename Type>
template <typename ValueType>
class SingleLinkedList<Type>::BasicIterator {
public:
    friend class SingleLinkedList;
    
    explicit BasicIterator(Node* node): node_(node) {}
    
public:

    using iterator_category = std::forward_iterator_tag;
    using value_type = Type;
    using difference_type = std::ptrdiff_t;
    using pointer = ValueType*;
    using reference = ValueType&;
    
    BasicIterator() = default;
    
    BasicIterator(const BasicIterator<Type>& other) noexcept {
        node_ = other.node_;
    }

    BasicIterator& operator=(const BasicIterator& rhs) = default;
    
    [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
        return node_ == rhs.node_;
    }
    
    [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
        return !(*this == rhs);
    }
    
    [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
        return node_ == rhs.node_;
    }
    
    [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
        return !(*this == rhs);
    }
    
    BasicIterator& operator++() noexcept {
        node_ = node_->next_node;
        return *this;
    }
    
    BasicIterator operator++(int) noexcept {
        auto old_value(*this);
        ++(*this);
        return old_value;
    }
    
    [[nodiscard]] reference operator*() const noexcept {
        return node_->value;
    }
    
    [[nodiscard]] pointer operator->() const noexcept {
        return &(node_->value);
    }
    
private:
    Node* node_ = nullptr;
};
