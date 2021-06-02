#include <cassert>
#include <stdexcept>
#include <string>
#include <iostream>

#include "scoped_pointer.h"

using namespace std;
using namespace std::literals;

// Этот main тестирует класс ScopedPtr
int main() {
    // Вспомогательный "шпион", позволяющий узнать о своём удалении
    struct DeletionSpy {
        explicit DeletionSpy(bool& is_deleted)
            : is_deleted_(is_deleted) {
        }
        ~DeletionSpy() {
            is_deleted_ = true;
        }
        bool& is_deleted_;
    };

    // Проверяем автоматическое удаление
    {
        bool is_deleted = false;
        {
            // настраиваем "шпион", чтобы при своём удалении он выставил is_deleted в true
            DeletionSpy* raw_ptr = new DeletionSpy(is_deleted);
            ScopedPtr<DeletionSpy> p(raw_ptr);
            assert(p.GetRawPtr() == raw_ptr);
            assert(!is_deleted);
            // При выходе из блока деструктор p должен удалить "шпиона"
        }
        // Если деструктор умного указателя работает правильно, шпион перед своей "смертью"
        // должен выставить is_deleted в true
        assert(is_deleted);
    }

    // Проверяем работу метода Release
    {
        bool is_deleted = false;
        DeletionSpy* raw_ptr = new DeletionSpy(is_deleted);
        {
            ScopedPtr<DeletionSpy> scoped_ptr(raw_ptr);
            assert(scoped_ptr.Release() == raw_ptr);
            assert(scoped_ptr.GetRawPtr() == nullptr);
            // После Release умный указатель не ссылается на объект и не удаляет его при своём удалении
        }
        assert(!is_deleted);
        delete raw_ptr;
        assert(is_deleted);
    }
    
    {
        // Для нулевого указателя приведение к типу bool возвращает false
        const ScopedPtr<int> empty_ptr;
        assert(!empty_ptr);

        // Для ненулевого указателя приведение к типу bool возвращает true
        const ScopedPtr<int> ptr_to_existing_object(new int(0));
        assert(ptr_to_existing_object);

        static_assert(noexcept(static_cast<bool>(ptr_to_existing_object)));
    }

    // Проверка работы оператора разыменования *
    {
        string* raw_ptr = new string("hello");
        ScopedPtr<string> smart_ptr(raw_ptr);
        // Ссылка, возвращаемая оператором разыменования должна ссылаться на объект,
        // на который указывает умный указатель
        assert(&*smart_ptr == raw_ptr);

        try {
            ScopedPtr<int> empty_ptr;
            // При попытке разыменовать пустой указатель должно быть выброшено
            // исключение logic_error
            *empty_ptr;
            // Сюда попасть мы не должны
            assert(false);
        } catch (const logic_error&) {
            // мы там, где нужно
        } catch (...) {
            // Других исключений выбрасываться не должно
            assert(false);
        }
    }

    // Проверка работы оператора ->
    {
        string* raw_ptr = new string("hello");
        ScopedPtr<string> smart_ptr(raw_ptr);
        // Доступ к членам класса через умный указатель должен быть аналогичен
        // доступу через "сырой" указатель
        assert(smart_ptr->data() == raw_ptr->data());

        try {
            ScopedPtr<string> empty_ptr;
            // При попытке разыменовать пустой указатель должно быть выброшено
            // исключение logic_error
            empty_ptr->clear();
            // Сюда попасть мы не должны
            assert(false);
        } catch (const logic_error&) {
            // мы там, где нужно
        } catch (...) {
            // Других исключений выбрасываться не должно
            assert(false);
        }
    }

    // Пример использования
    {
        // На этой структуре будет проверяться работа умного указателя
        struct Object {
            Object() {
                cout << "Object is default constructed"s << endl;
            }
            void DoSomething() {
                cout << "Doing something"s << endl;
            }
            ~Object() {
                cout << "Object is destroyed"s << endl;
            }
        };

        // Сконструированный по умолчанию указатель ссылается на nullptr
        ScopedPtr<Object> empty_smart_ptr;
        // Перегруженный оператор приведения к типу bool вернёт false для пустого указателя
        assert(!empty_smart_ptr);

        ScopedPtr<Object> smart_ptr(new Object());
        // Перегруженный оператор bool вернёт true для указателя, ссылающегося на объект
        assert(smart_ptr);

        // Проверка оператора разыменования
        (*smart_ptr).DoSomething();
        // Проверка оператора доступа к членам класса
        smart_ptr->DoSomething();
    }
}
