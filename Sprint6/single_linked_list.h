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
    
    SingleLinkedList& operator=(const SingleLinkedList& right);
    
public:
    [[nodiscard]] size_t GetSize() const noexcept;
    
    [[nodiscard]] bool IsEmpty() const noexcept;
    
    void PushFront(const Type& value);
    
    void Clear() noexcept;
    
    void swap(SingleLinkedList& other) noexcept;
    
    Iterator InsertAfter(ConstIterator before_inserted, const Type& value);

    void PopFront() noexcept;

    Iterator EraseAfter(ConstIterator before_deleted) noexcept;

public:
    [[nodiscard]] Iterator begin() noexcept;
    
    [[nodiscard]] Iterator end() noexcept;
    
    [[nodiscard]] ConstIterator begin() const noexcept;
    
    [[nodiscard]] ConstIterator end() const noexcept;
    
    [[nodiscard]] ConstIterator cbegin() const noexcept;
    
    [[nodiscard]] ConstIterator cend() const noexcept;
    
    [[nodiscard]] Iterator before_begin() noexcept;

    [[nodiscard]] ConstIterator cbefore_begin() const noexcept;

    [[nodiscard]] ConstIterator before_begin() const noexcept;
    
private:
    struct Node;
    
private:
    template <typename ForwardIterator>
    SingleLinkedList(ForwardIterator begin, ForwardIterator end);
    
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

// private range based constructor
template <typename Type>
template <typename ForwardIterator>
SingleLinkedList<Type>::SingleLinkedList(ForwardIterator begin, ForwardIterator end) {
    assert(size_ == 0 && head_.next_node == nullptr);
    
    auto last_node = before_begin_;
    
    for (auto it = begin; it != end; ++it) {
        InsertAfter(last_node, *it);
        ++last_node;
    }
}

// destructor
template <typename Type>
SingleLinkedList<Type>::~SingleLinkedList() {
    Clear();
}

// operator=
template <typename Type>
SingleLinkedList<Type>& SingleLinkedList<Type>::operator=(const SingleLinkedList& right) {
    if (this != &right) {
        SingleLinkedList temp(right);
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
void swap(SingleLinkedList<Type>& left, SingleLinkedList<Type>& right) noexcept {
    left.swap(right);
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

// EraseAfter
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

// iterator access
template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::Iterator SingleLinkedList<Type>::begin() noexcept {
    auto before_begin_copy = before_begin_;
    return ++before_begin_copy;
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::Iterator SingleLinkedList<Type>::end() noexcept {
    return end_;
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::ConstIterator SingleLinkedList<Type>::begin() const noexcept {
    auto before_begin_copy = before_begin_;
    return ++before_begin_copy;
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::ConstIterator SingleLinkedList<Type>::end() const noexcept {
    return end_;
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::ConstIterator SingleLinkedList<Type>::cbegin() const noexcept {
    return begin();
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::ConstIterator SingleLinkedList<Type>::cend() const noexcept {
    return end();
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::Iterator SingleLinkedList<Type>::before_begin() noexcept {
    return before_begin_;
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::ConstIterator SingleLinkedList<Type>::cbefore_begin() const noexcept {
    return before_begin_;
}

template <typename Type>
[[nodiscard]] typename SingleLinkedList<Type>::ConstIterator SingleLinkedList<Type>::before_begin() const noexcept {
    return before_begin_;
}

// Node
template <typename Type>
struct SingleLinkedList<Type>::Node {
    Node() = default;
    
    Node(const Type& val, Node* next)
    : value(val)
    , next_node(next) {
    }
    
    Type value;
    Node* next_node = nullptr;
};

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

    BasicIterator& operator=(const BasicIterator& right) = default;
    
public:
    [[nodiscard]] bool operator==(const BasicIterator<const Type>& right) const noexcept {
        return node_ == right.node_;
    }
    
    [[nodiscard]] bool operator==(const BasicIterator<Type>& right) const noexcept {
        return node_ == right.node_;
    }
    
    [[nodiscard]] bool operator!=(const BasicIterator<const Type>& right) const noexcept {
        return !(*this == right);
    }
    
    [[nodiscard]] bool operator!=(const BasicIterator<Type>& right) const noexcept {
        return !(*this == right);
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
bool operator==(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return std::equal(left.begin(), left.end(), right.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return !(left == right);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
     return !(right < left);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return right < left;
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return right <= left;
}
