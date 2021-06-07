#pragma once

#include <iterator>
#include <cstddef>
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
    
    SingleLinkedList(std::initializer_list<Type> values);
    
    SingleLinkedList(const SingleLinkedList<Type>& other);
    
    ~SingleLinkedList();
    
    SingleLinkedList& operator=(const SingleLinkedList& rhs);
    
public:
    [[nodiscard]] size_t GetSize() const noexcept;
    
    [[nodiscard]] bool IsEmpty() const noexcept;
    
    void PushFront(const Type& value);
    
    void Clear() noexcept;
    
    void swap(SingleLinkedList& other) noexcept;
    
    Iterator InsertAfter(ConstIterator pos, const Type& value);

    void PopFront() noexcept;

    Iterator EraseAfter(ConstIterator pos) noexcept;

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
    
    [[nodiscard]] Iterator before_begin() noexcept {
        return before_begin_;
    }

    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        return before_begin_;
    }

    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return before_begin_;
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
        assert(size_ == 0 && head_.next_node == nullptr);
        
        auto last_node = before_begin_;
        
        for (auto it = begin; it != end; ++it) {
            InsertAfter(last_node, *it);
            ++last_node;
        }
    }
    
private:
    Node head_{};
    size_t size_ = 0;
    
    Iterator before_begin_{&head_};
    Iterator end_{head_.next_node};
};

// initializer_list constructor
template <typename Type>
SingleLinkedList<Type>::SingleLinkedList(std::initializer_list<Type> values) {
    SingleLinkedList temp(values.begin(), values.end());
    swap(temp);
}

// copy constructor
template <typename Type>
SingleLinkedList<Type>::SingleLinkedList(const SingleLinkedList<Type>& other) {
    SingleLinkedList temp(other.begin(), other.end());
    swap(temp);
}

// destructor
template <typename Type>
SingleLinkedList<Type>::~SingleLinkedList() {
    Clear();
}

// operator=
template <typename Type>
SingleLinkedList<Type>& SingleLinkedList<Type>::operator=(const SingleLinkedList& rhs) {
    if (this != &rhs) {
        SingleLinkedList temp(rhs);
        swap(temp);
    }

    return *this;
}

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

// InsertAfter
template <typename Type>
typename SingleLinkedList<Type>::Iterator SingleLinkedList<Type>::InsertAfter(ConstIterator before_inserted, const Type& value) {
    auto node = before_inserted.GetRawPointer();
    
    if (node == nullptr) {
        PushFront(value);
        return begin();
    }
    
    const auto node_after_inserted = node->next_node;
    
    Node* new_node = new Node{value, node_after_inserted};
    
    node->next_node = new_node;
    
    ++size_;
    
    return Iterator{new_node};
}

// PopFront
template <typename Type>
void SingleLinkedList<Type>::PopFront() noexcept {
    if (IsEmpty()) {
        return;
    }
    
    auto node_to_delete = head_.next_node;
    head_.next_node = head_.next_node->next_node;
    delete node_to_delete;
}


template <typename Type>
typename SingleLinkedList<Type>::Iterator SingleLinkedList<Type>::EraseAfter(ConstIterator before_deleted) noexcept {
    auto node = before_deleted.GetRawPointer();
    
    if (node == nullptr) {
        return Iterator{};
    }
    
    auto node_to_delete = node->next_node;
    
    if (node_to_delete == nullptr) {
        return Iterator{};
    }
    
    node->next_node = node->next_node->next_node;
    
    delete node_to_delete;
    
    --size_;
    
    return Iterator{node->next_node};
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
