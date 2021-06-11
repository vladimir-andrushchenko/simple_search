#pragma once

#include <iterator>
#include <cstddef>
#include <algorithm>
#include <cassert>

template <typename Type>
class SingleLinkedList {
public:
    struct Node {
        Node() = default;
        
        Node(const Type& node_value, Node* next)
        : value(node_value)
        , next_node(next) {
        }
        
        Type value;
        Node* next_node = nullptr;
    };
    
    template <typename ValueType>
    class BasicIterator {
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
    
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;
    
public:
    SingleLinkedList() = default;
    
    SingleLinkedList(std::initializer_list<Type> values) {
        Assign(values.begin(), values.end());
    }
    
    SingleLinkedList(const SingleLinkedList<Type>& other)  {
        Assign(other.begin(), other.end());
    }
    
    ~SingleLinkedList() {
        Clear();
    }
    
    SingleLinkedList& operator=(const SingleLinkedList& right) {
        Assign(right.begin(), right.end());
        
        return *this;
    }
    
public:
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }
    
    [[nodiscard]] bool IsEmpty() const noexcept  {
        return size_ == 0;
    }
    
    void PushFront(const Type& value)  {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }
    
    void Clear() noexcept  {
        while (head_.next_node) {
            Node* current_node = head_.next_node;
            head_.next_node = current_node->next_node;
            delete current_node;
            
            --size_;
        }
    }
    
    void swap(SingleLinkedList& other) noexcept  {
        std::swap(head_.next_node, other.head_.next_node);
        std::swap(size_, other.size_);
    }
    
    Iterator InsertAfter(ConstIterator before_inserted, const Type& value)  {
        auto node = before_inserted.GetRawPointer();
        
        assert(node != nullptr);
        
        const auto node_after_inserted = node->next_node;
        
        Node* new_node = new Node{value, node_after_inserted};
        
        node->next_node = new_node;
        
        ++size_;
        
        return Iterator{new_node};
    }
    
    void PopFront() noexcept {
        if (IsEmpty()) {
            return;
        }
        
        auto node_to_delete = head_.next_node;
        head_.next_node = head_.next_node->next_node;
        delete node_to_delete;
        
        --size_;
    }
    
    Iterator EraseAfter(ConstIterator before_deleted) noexcept  {
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
    
public:
    [[nodiscard]] Iterator begin() noexcept  {
        return Iterator{head_.next_node};
    }
    
    [[nodiscard]] Iterator end() noexcept {
        return Iterator{nullptr};
    }
    
    [[nodiscard]] ConstIterator begin() const noexcept {
        return ConstIterator{const_cast<Node *>(head_.next_node)};
    }
    
    [[nodiscard]] ConstIterator end() const noexcept {
        return ConstIterator{nullptr};
    }
    
    [[nodiscard]] ConstIterator cbegin() const noexcept  {
        return begin();
    }
    
    [[nodiscard]] ConstIterator cend() const noexcept  {
        return end();
    }
    
    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator{&head_};
    }
    
    [[nodiscard]] ConstIterator cbefore_begin() const noexcept  {
        return ConstIterator{const_cast<Node *>(&head_)};
    }
    
    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return ConstIterator{const_cast<Node *>(&head_)};
    }
    
private:
    template <typename InputIterator>
    void Assign(InputIterator from, InputIterator to) {
        SingleLinkedList temp;
        
        auto last_node = temp.before_begin();
        
        for (auto it = from; it != to; ++it) {
            temp.InsertAfter(last_node, *it);
            ++last_node;
        }
        
        swap(temp);
    }
    
private:
    Node head_{};
    size_t size_ = 0;
};

// swap
template <typename Type>
void swap(SingleLinkedList<Type>& left, SingleLinkedList<Type>& right) noexcept {
    left.swap(right);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    if (&left == &right) {
        return true;
    }
    
    if (left.GetSize() != right.GetSize()) {
        return false;
    }
    
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
