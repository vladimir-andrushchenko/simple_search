#include <iostream>
#include <vector>
#include <numeric>
#include <stack>

using namespace std;

template <typename It>
void PrintRange(It range_begin, It range_end) {
    for (auto it = range_begin; it != range_end; ++it) {
        cout << *it << " "s;
    }
    cout << endl;
}

template <typename Type>
class Stack {
public:
    void Push(const Type& element) {
        elements_.push_back(element);
    }
    void Pop() {
        elements_.pop_back();
    }
    const Type& Peek() const {
        return elements_.back();
    }
    Type& Peek() {
        return elements_.back();
    }
    void Print() const {
        PrintRange(elements_.begin(), elements_.end());
    }
    uint64_t Size() const {
        return elements_.size();
    }
    bool IsEmpty() const {
        return elements_.empty();
    }

private:
    vector<Type> elements_;
};

template <typename Type>
class StackMin {
public:
    void Push(const Type& element) {
        elements_.Push(element);
        if (min_elements_.IsEmpty()) {
            min_elements_.Push(element);
        } else {
            const auto top_min_element = min_elements_.Peek();
            if (top_min_element < element) {
                min_elements_.Push(top_min_element);
            } else {
                min_elements_.Push(element);
            }
        }
    }
    void Pop() {
        elements_.Pop();
        min_elements_.Pop();
    }
    const Type& Peek() const {
        return elements_.Peek();
    }
    Type& Peek() {
        return elements_.Peek();
    }
    void Print() const {
        // работу этого метода мы проверять не будем,
        // но если он вам нужен, то можете его реализовать
        elements_.Print();
    }
    uint64_t Size() const {
        return elements_.size();
    }
    bool IsEmpty() const {
        return elements_.IsEmpty();
    }
    const Type& PeekMin() const {
        return min_elements_.Peek();
    }
    Type& PeekMin() {
        return min_elements_.Peek();
    }
private:
    Stack<Type> elements_;
    Stack<Type> min_elements_;
};

template <typename Type>
class SortedStack {
public:
    void Push(const Type& element) {
        if (elements_.IsEmpty()) {
            elements_.Push(element);
            return;
        }
        
        Stack<Type> temporary_stack;
        
        auto top_element = elements_.Peek();
        
        while (true) {
            if (top_element > element) {
                elements_.Push(element);
                break;
            }
            
            temporary_stack.Push(top_element);
            
            elements_.Pop();
            
            if (elements_.IsEmpty()) {
                elements_.Push(element);
                break;
            }
            
            top_element = elements_.Peek();
        }
                
        
        while (!temporary_stack.IsEmpty()) {
            elements_.Push(temporary_stack.Peek());
            temporary_stack.Pop();
        }
    }
    
    void Pop() {
        elements_.Pop();
    }
    
    const Type& Peek() const {
    return elements_.Peek();
    }
    
    Type& Peek() {
    return elements_.Peek();
    }
    
    void Print() const {
    elements_.Print();
    }
    
    uint64_t Size() const {
    return elements_.Size();
    }
    
    bool IsEmpty() const {
    return elements_.IsEmpty();
    }
    
private:
    Stack<Type> elements_;
};

template <typename Type>
class Queue {
public:
    void Push(const Type& element) {
        // просто вставим элемент в первый стек
        stack1_.push(element);
    }
    void Pop() {
        // переложим все элементы из первого стека во второй,
        // так что первый элемент очереди окажется на вершине второго стека
        MoveElements();
        // вытащим верхний элемент второго стека
        stack2_.pop();
    }
    Type& Front() {
        // логика аналогична методу Pop
        MoveElements();
        return stack2_.top();
    }
    const Type& Front() const {
        // логика аналогична методу Pop
        MoveElements();
        return stack2_.top();
    }
    uint64_t Size() const {
        return stack1_.size() + stack2_.size();
    }
    bool IsEmpty() const {
        return (stack1_.empty() && stack2_.empty());
    }

private:
    stack<Type> stack1_;
    stack<Type> stack2_;

    void MoveElements() {
        // Перекладывать во второй стек из первого
        // необходимо только тогда, когда во втором стеке
        // элементов не осталось. Иначе порядок очереди будет нарушен.
        if (stack2_.empty()) {
            while (!stack1_.empty()) {
                stack2_.push(stack1_.top());
                stack1_.pop();
            }
        }
    }
};

int main() {
    Queue<int> queue;
    vector<int> values(5);

    // заполняем вектор для тестирования очереди
    iota(values.begin(), values.end(), 1);
    // перемешиваем значения
    random_shuffle(values.begin(), values.end());

    PrintRange(values.begin(), values.end());

    cout << "Заполняем очередь"s << endl;

    // заполняем очередь и выводим элемент в начале очереди
    for (int i = 0; i < 5; ++i) {
        queue.Push(values[i]);
        cout << "Вставленный элемент "s << values[i] << endl;
        cout << "Первый элемент очереди "s << queue.Front() << endl;
    }

    cout << "Вынимаем элементы из очереди"s << endl;

    // выводим элемент в начале очереди и вытаскиваем элементы по одному
    while (!queue.IsEmpty()) {
        // сначала будем проверять начальный элемент, а потом вытаскивать,
        // так как операция Front на пустой очереди не определена
        cout << "Будем вынимать элемент "s << queue.Front() << endl;
        queue.Pop();
    }
    return 0;
}
