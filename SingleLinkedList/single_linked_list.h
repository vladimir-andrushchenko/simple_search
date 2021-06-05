#pragma once

#include <iterator>
#include <cstddef>
#include <vector>
#include <forward_list>
#include <algorithm>
#include <cassert>

using namespace std;

template <typename Type>
class SingleLinkedList {
public:
    template <typename ValueType>
    class BasicIterator;
    
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;
    
public:
    SingleLinkedList() = default;
    
    SingleLinkedList(std::initializer_list<Type> values) {
        SingleLinkedList temp(values.begin(), values.end());
        swap(temp);
    }
    
    SingleLinkedList(const SingleLinkedList<Type>& other) {
        SingleLinkedList temp(other.begin(), other.end());
        swap(temp);
    }
    
    ~SingleLinkedList() {
        Clear();
    }
    
    SingleLinkedList& operator=(const SingleLinkedList& rhs) {
        if (this != &rhs) {
            SingleLinkedList temp(rhs);
            swap(temp);
        }

        return *this;
    }
    
public:
    [[nodiscard]] size_t GetSize() const noexcept;
    
    [[nodiscard]] bool IsEmpty() const noexcept;
    
    void PushFront(const Type& value);
    
    void Clear() noexcept;
    
    void swap(SingleLinkedList& other) noexcept;

public:
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
    
    // Возвращает итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] Iterator before_begin() noexcept {
        return before_begin_;
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        return before_begin_;
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return before_begin_;
    }
    
public:
    /*
     * Вставляет элемент value после элемента, на который указывает pos.
     * Возвращает итератор на вставленный элемент
     * Если при создании элемента будет выброшено исключение, список останется в прежнем состоянии
     */
    Iterator InsertAfter(ConstIterator pos, const Type& value) {
        auto node = pos.GetRawPointer();
        
        if (node == nullptr) {
            PushFront(value);
        }
        
        const auto node_after_inserted = node->next_node;
        
        Node* new_node = new Node{value, node_after_inserted};
        
        node->next_node = new_node;
        
        ++size_;
        
        return Iterator{new_node};
    }

    void PopFront() noexcept {
        auto node_to_delete = head_.next_node;
        head_.next_node = head_.next_node->next_node;
        delete node_to_delete;
    }

    /*
     * Удаляет элемент, следующий за pos.
     * Возвращает итератор на элемент, следующий за удалённым
     */
    Iterator EraseAfter(ConstIterator pos) noexcept {
        // Заглушка. Реализуйте метод самостоятельно
        return {};
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
    template <typename Iterator>
    SingleLinkedList(Iterator begin, Iterator end) {
        // Сначала надо удостовериться, что текущий список пуст
        assert(size_ == 0 && head_.next_node == nullptr);
        
        auto last_node = before_begin_;
        
        for (auto it = begin; it != end; ++it) {
            InsertAfter(last_node, *it);
            ++last_node;
        }
    }
    
//private:
//    void InsertAfter(Node* node, const Type& value) {
//        if (node == nullptr) {
//            PushFront(value);
//        }
//
//        const auto node_after_inserted = node->next_node;
//
//        Node* new_node = new Node{value, node_after_inserted};
//
//        node->next_node = new_node;
//    }
    
private:
    Node head_{};
    size_t size_ = 0;
    
    Iterator before_begin_{&head_};
    Iterator end_{head_.next_node};
};

// GetSize
template <typename Type>
[[nodiscard]] size_t SingleLinkedList<Type>::GetSize() const noexcept {
    return size_;
}

// IsEmpty
template <typename Type>
[[nodiscard]] bool SingleLinkedList<Type>::IsEmpty() const noexcept {
    return size_ == 0;
}

// PushFront
template <typename Type>
void SingleLinkedList<Type>::PushFront(const Type& value) {
    head_.next_node = new Node(value, head_.next_node);
    ++size_;
}

// Clear
template <typename Type>
void SingleLinkedList<Type>::Clear() noexcept {
    while (head_.next_node) {
        Node* current_node = head_.next_node;
        head_.next_node = current_node->next_node;
        delete current_node;
        
        --size_;
    }
}

// swap
template <typename Type>
void SingleLinkedList<Type>::swap(SingleLinkedList& other) noexcept {
    auto temp = head_.next_node;
    head_.next_node = other.head_.next_node;
    other.head_.next_node = temp;
    std::swap(size_, other.size_);
}

// swap
template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

// BasicIterator
template <typename Type>
template <typename ValueType>
class SingleLinkedList<Type>::BasicIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Type;
    using difference_type = std::ptrdiff_t;
    using pointer = ValueType*;
    using reference = ValueType&;
    
public:
    friend class SingleLinkedList;
    
    BasicIterator() = default;
    
    BasicIterator(const BasicIterator<Type>& other) noexcept {
        node_ = other.node_;
    }
    
    explicit BasicIterator(Node* node): node_(node) {}

    BasicIterator& operator=(const BasicIterator& rhs) = default;
    
public:
    [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
        return node_ == rhs.node_;
    }
    
    [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
        return node_ == rhs.node_;
    }
    
    [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
        return !(*this == rhs);
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
    
public:
    auto GetRawPointer() {
        return node_;
    }
    
private:
    Node* node_ = nullptr;
};

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
     return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return rhs <= lhs;
}
