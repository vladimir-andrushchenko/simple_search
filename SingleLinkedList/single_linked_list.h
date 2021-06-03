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
    
    // Итератор, допускающий изменение элементов списка
    using Iterator = BasicIterator<Type>;
    // Константный итератор, предоставляющий доступ для чтения к элементам списка
    using ConstIterator = BasicIterator<const Type>;
    
    // Возвращает итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен end()
    [[nodiscard]] Iterator begin() noexcept {
        assert(false);
        // Реализуйте самостоятельно
        return {};
    }
    
    // Возвращает итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] Iterator end() noexcept {
        assert(false);
        // Реализуйте самостоятельно
        return {};
    }
    
    // Возвращает константный итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен end()
    // Результат вызова эквивалентен вызову метода cbegin()
    [[nodiscard]] ConstIterator begin() const noexcept {
        assert(false);
        // Реализуйте самостоятельно
        return {};
    }
    
    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    // Результат вызова эквивалентен вызову метода cend()
    [[nodiscard]] ConstIterator end() const noexcept {
        assert(false);
        // Реализуйте самостоятельно
        return {};
    }
    
    // Возвращает константный итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен cend()
    [[nodiscard]] ConstIterator cbegin() const noexcept {
        assert(false);
        // Реализуйте самостоятельно
        return {};
    }
    
    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator cend() const noexcept {
        assert(false);
        // Реализуйте самостоятельно
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
    Node head_{};
    size_t size_ = 0;
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
    // Класс списка объявляется дружественным, чтобы из методов списка
    // был доступ к приватной области итератора
    friend class SingleLinkedList;
    
    // Конвертирующий конструктор итератора из указателя на узел списка
    explicit BasicIterator(Node* node): node_(node) {}
    
public:
    // Объявленные ниже типы сообщают стандартной библиотеке о свойствах этого итератора
    
    // Категория итератора - forward iterator
    // (итератор, который поддерживает операции инкремента и многократное разыменование)
    using iterator_category = std::forward_iterator_tag;
    // Тип элементов, по которым перемещается итератор
    using value_type = Type;
    // Тип, используемый для хранения смещения между итераторами
    using difference_type = std::ptrdiff_t;
    // Тип указателя на итерируемое значение
    using pointer = ValueType*;
    // Тип ссылки на итерируемое значение
    using reference = ValueType&;
    
    BasicIterator() = default;
    
    // Конвертирующий конструктор/конструктор копирования
    // При ValueType, совпадающем с Type, играет роль копирующего конструктора
    // При ValueType, совпадающем с const Type, играет роль конвертирующего конструктора
    BasicIterator(const BasicIterator<Type>& other) noexcept {
        assert(false);
        // Реализуйте конструктор самостоятельно
    }
    
    // Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
    // пользовательского конструктора копирования, явно объявим оператор = и
    // попросим компилятор сгенерировать его за нас.
    BasicIterator& operator=(const BasicIterator& rhs) = default;
    
    // Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
    // Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
    [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
        assert(false);
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
    // Оператор проверки итераторов на неравенство
    // Противоположен !=
    [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
        assert(false);
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
    // Оператор сравнения итераторов (в роли второго аргумента итератор)
    // Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
    [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
        assert(false);
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
    // Оператор проверки итераторов на неравенство
    // Противоположен !=
    [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
        assert(false);
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
    // Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
    // Возвращает ссылку на самого себя
    // Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
    BasicIterator& operator++() noexcept {
        assert(false);
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
    // Оператор постинкремента. После его вызова итератор указывает на следующий элемент списка.
    // Возвращает прежнее значение итератора
    // Инкремент итератора, не указывающего на существующий элемент списка,
    // приводит к неопределённому поведению
    BasicIterator operator++(int) noexcept {
        assert(false);
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
    // Операция разыменования. Возвращает ссылку на текущий элемент
    // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
    // приводит к неопределённому поведению
    [[nodiscard]] reference operator*() const noexcept {
        assert(false);
        // Не реализовано
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
    // Операция доступа к члену класса. Возвращает указатель на текущий элемент списка.
    // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
    // приводит к неопределённому поведению
    [[nodiscard]] pointer operator->() const noexcept {
        assert(false);
        // Заглушка. Реализуйте оператор самостоятельно
    }
    
private:
    Node* node_ = nullptr;
};
