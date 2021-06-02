#include <iostream>
//#include <vector>

#include "ptr_vector.h"

using namespace std;

// Эта функция main тестирует шаблон класса PtrVector
int main() {
    struct CopyingSpy {
        CopyingSpy(int& copy_count, int& deletion_count)
            : copy_count_(copy_count)
            , deletion_count_(deletion_count) {
        }
        CopyingSpy(const CopyingSpy& rhs)
            : copy_count_(rhs.copy_count_)          // счётчик копирований
            , deletion_count_(rhs.deletion_count_)  // счётчик удалений
        {
            if (rhs.throw_on_copy_) {
                throw runtime_error("copy construction failed"s);
            }
            ++copy_count_;
        }
        ~CopyingSpy() {
            ++deletion_count_;
        }
        void ThrowOnCopy() {
            throw_on_copy_ = true;
        }

    private:
        int& copy_count_;
        int& deletion_count_;
        bool throw_on_copy_ = false;
    };

    // Проверка присваивания
    {
        int item0_copy_count = 0;
        int item0_deletion_count = 0;
        {
            PtrVector<CopyingSpy> v;

            v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
            v.GetItems().push_back(nullptr);
            {
                PtrVector<CopyingSpy> v_copy;
                v_copy = v;
                assert(v_copy.GetItems().size() == v.GetItems().size());
                assert(v_copy.GetItems().at(0) != v.GetItems().at(0));
                assert(v_copy.GetItems().at(1) == nullptr);
                assert(item0_copy_count == 1);
                assert(item0_deletion_count == 0);
            }
            assert(item0_deletion_count == 1);
        }
        assert(item0_deletion_count == 2);
    }

    // Проверка корректности самоприсваивания
    {
        int item0_copy_count = 0;
        int item0_deletion_count = 0;

        PtrVector<CopyingSpy> v;
        v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
        CopyingSpy* first_item = v.GetItems().front();

        v = v;
        assert(v.GetItems().size() == 1);
        // При самоприсваивании объекты должны быть расположены по тем же адресам
        assert(v.GetItems().front() == first_item);
        assert(item0_copy_count == 0);
        assert(item0_deletion_count == 0);
    }

    // Проверка обеспечения строгой гарантии безопасности исключений при присваивании
    {
        int item0_copy_count = 0;
        int item0_deletion_count = 0;

        int item1_copy_count = 0;
        int item1_deletion_count = 0;

        // v хранит 2 элемента
        PtrVector<CopyingSpy> v;
        v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
        v.GetItems().push_back(new CopyingSpy(item1_copy_count, item1_deletion_count));

        int other_item0_copy_count = 0;
        int other_item0_deletion_count = 0;
        // other_vector хранит 1 элемент, при копировании которого будет выброшено исключение
        PtrVector<CopyingSpy> other_vector;
        other_vector.GetItems().push_back(new CopyingSpy(other_item0_copy_count, other_item0_deletion_count));
        other_vector.GetItems().front()->ThrowOnCopy();

        // Сохраняем массив указателей
        auto v_items(v.GetItems());

        try {
            v = other_vector;
            // Операция должна выбросить исключение
            assert(false);
        } catch (const runtime_error&) {
        }

        // Элементы массива должны остаться прежними
        assert(v.GetItems() == v_items);
        assert(item0_copy_count == 0);
        assert(item1_copy_count == 0);
        assert(other_item0_copy_count == 0);
    }
}

/*
// Эта функция main тестирует шаблон класса PtrVector
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

    // Проверяем удаление элементов
    {
        bool spy1_is_deleted = false;
        DeletionSpy* ptr1 = new DeletionSpy(spy1_is_deleted);
        {
            PtrVector<DeletionSpy> ptr_vector;
            ptr_vector.GetItems().push_back(ptr1);
            assert(!spy1_is_deleted);

            // Константная ссылка на ptr_vector
            const auto& const_ptr_vector_ref(ptr_vector);
            // И константная, и неконстантная версия GetItems
            // должны вернуть ссылку на один и тот же вектор
            assert(&const_ptr_vector_ref.GetItems() == &ptr_vector.GetItems());
        }
        // При разрушении ptr_vector должен удалить все объекты, на которые
        // ссылаются находящиеся внутри него указателели
        assert(spy1_is_deleted);
    }

    // Вспомогательный «шпион», позволяющий узнать о своём копировании
    struct CopyingSpy {
        explicit CopyingSpy(int& copy_count)
            : copy_count_(copy_count) {
        }
        CopyingSpy(const CopyingSpy& rhs)
            : copy_count_(rhs.copy_count_)  //
        {
            ++copy_count_;
        }
        int& copy_count_;
    };

    // Проверяем копирование элементов при копировании массива указателей
    {
        // 10 элементов
        vector<int> copy_counters(10);

        PtrVector<CopyingSpy> ptr_vector;
        // Подготавливаем оригинальный массив указателей
        for (auto& counter : copy_counters) {
            ptr_vector.GetItems().push_back(new CopyingSpy(counter));
        }
        // Последний элемент содержит нулевой указатель
        ptr_vector.GetItems().push_back(nullptr);

        auto ptr_vector_copy(ptr_vector);
        // Количество элементов в копии равно количеству элементов оригинального вектора
        assert(ptr_vector_copy.GetItems().size() == ptr_vector.GetItems().size());

        // копия должна хранить указатели на новые объекты
        assert(ptr_vector_copy.GetItems() != ptr_vector.GetItems());
        // Последний элемент исходного массива и его копии - нулевой указатель
        assert(ptr_vector_copy.GetItems().back() == nullptr);
        // Проверяем, что элементы были скопированы (копирующие шпионы увеличивают счётчики копий).
        assert(all_of(copy_counters.begin(), copy_counters.end(), [](int counter) {
            return counter == 1;
        }));
    }
    
    std::cout << "seems to work" << std::endl;
}
*/
