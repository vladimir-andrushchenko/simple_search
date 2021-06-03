#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "ptr_vector.h"
#include "scoped_pointer.h"

using namespace std;

// Щупальце
class Tentacle {
public:
    explicit Tentacle(int id) noexcept
        : id_(id) {
    }

    int GetId() const noexcept {
        return id_;
    }

    Tentacle* GetLinkedTentacle() const noexcept {
        return linked_tentacle_;
    }
    void LinkTo(Tentacle& tentacle) noexcept {
        linked_tentacle_ = &tentacle;
    }
    void Unlink() noexcept {
        linked_tentacle_ = nullptr;
    }

private:
    int id_ = 0;
    Tentacle* linked_tentacle_ = nullptr;
};

// Осьминог
class Octopus {
public:
    Octopus()
        : Octopus(8) {
    }
    
    explicit Octopus(int num_tentacles) {
        for (int i = 1; i <= num_tentacles; ++i) {
            ScopedPtr<Tentacle> ptr(new Tentacle(i));
            tentacles_.push_back(ptr.Release());
        }
    }
    
    // Добавляет новое щупальце с идентификатором,
    // равным (количество_щупалец + 1):
    // 1, 2, 3, ...
    // Возвращает ссылку на добавленное щупальце
     Tentacle& AddTentacle() {
         tentacles_.push_back(new Tentacle(static_cast<int>(tentacles_.size()) + 1));
         return *tentacles_.GetItems().back();
     }

    int GetTentacleCount() const noexcept {
        return static_cast<int>(tentacles_.size());
    }

    const Tentacle& GetTentacle(size_t index) const {
        return *tentacles_.at(index);
    }
    Tentacle& GetTentacle(size_t index) {
        return *tentacles_.at(index);
    }

private:

    // Вектор хранит указатели на щупальца. Сами объекты щупалец находятся в куче
    PtrVector<Tentacle> tentacles_;
};

int main() {
    // Проверка присваивания осьминогов
        {
            Octopus octopus1(3);
            // Настраиваем состояние исходного осьминога
            octopus1.GetTentacle(2).LinkTo(octopus1.GetTentacle(1));

            // До присваивания octopus2 имеет своё собственное состояние
            Octopus octopus2(10);

            octopus2 = octopus1;

            // После присваивания осьминогов щупальца копии имеют то же состояние,
            // что и щупальца присваиваемого объекта
            assert(octopus2.GetTentacleCount() == octopus1.GetTentacleCount());
            for (int i = 0; i < octopus2.GetTentacleCount(); ++i) {
                auto& tentacle1 = octopus1.GetTentacle(i);
                auto& tentacle2 = octopus2.GetTentacle(i);
                assert(&tentacle2 != &tentacle1);
                assert(tentacle2.GetId() == tentacle1.GetId());
                assert(tentacle2.GetLinkedTentacle() == tentacle1.GetLinkedTentacle());
            }
        }

        // Проверка самоприсваивания осьминогов
        {
            Octopus octopus(3);
            // Настраиваем состояние осьминога
            octopus.GetTentacle(0).LinkTo(octopus.GetTentacle(1));

            vector<pair<Tentacle*, Tentacle*>> tentacles;
            // Сохраняем информацию о щупальцах осьминога и его копии
            for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
                tentacles.push_back({&octopus.GetTentacle(i), octopus.GetTentacle(i).GetLinkedTentacle()});
            }

            // Выполняем самоприсваивание
            octopus = octopus;

            // После самоприсваивания состояние осьминога не должно измениться
            assert(octopus.GetTentacleCount() == static_cast<int>(tentacles.size()));
            for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
                auto& tentacle_with_link = tentacles.at(i);
                assert(&octopus.GetTentacle(i) == tentacle_with_link.first);
                assert(octopus.GetTentacle(i).GetLinkedTentacle() == tentacle_with_link.second);
            }
        }
    
    // Проверка конструирования осьминогов
    {
        // По умолчанию осьминог имеет 8 щупалец
        Octopus default_octopus;
        assert(default_octopus.GetTentacleCount() == 8);

        // Осьминог может иметь отличное от 8 количество щупалец
        Octopus quadropus(4);
        assert(quadropus.GetTentacleCount() == 4);

        // И даже вообще не иметь щупалец
        Octopus coloboque(0);
        assert(coloboque.GetTentacleCount() == 0);
    }

    // Осьминогу можно добавлять щупальца
    // Раскомментируйте код, после того как реализуете метод AddTentacle
    
    {
        Octopus octopus(1);
        Tentacle* t0 = &octopus.GetTentacle(0);
        Tentacle* t1 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 2);
        Tentacle* t2 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 3);

        // После добавления щупалец ранее созданные щупальца не меняют своих адресов
        assert(&octopus.GetTentacle(0) == t0);
        assert(&octopus.GetTentacle(1) == t1);
        assert(&octopus.GetTentacle(2) == t2);

        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            assert(octopus.GetTentacle(i).GetId() == i + 1);
        }
    }
    

    // Осьминоги могут прицепляться к щупальцам друг друга
    {
        Octopus male(2);
        Octopus female(2);

        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);

        male.GetTentacle(0).LinkTo(female.GetTentacle(1));
        assert(male.GetTentacle(0).GetLinkedTentacle() == &female.GetTentacle(1));

        male.GetTentacle(0).Unlink();
        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);
    }

    // Копия осьминога имеет свою собственную копию щупалец, которые
    // копируют состояние щупалец оригинального осьминога
    {
        // Перебираем осьминогов с разным количеством щупалец
        for (int num_tentacles = 0; num_tentacles < 10; ++num_tentacles) {
            Octopus male(num_tentacles);
            Octopus female(num_tentacles);
            // Пусть они хватают друг друга за щупальца
            for (int i = 0; i < num_tentacles; ++i) {
                male.GetTentacle(i).LinkTo(female.GetTentacle(num_tentacles - 1 - i));
            }

            Octopus male_copy(male);
            // Проверяем состояние щупалец копии
            assert(male_copy.GetTentacleCount() == male.GetTentacleCount());
            for (int i = 0; i < male_copy.GetTentacleCount(); ++i) {
                // Каждое щупальце копии размещается по адресу, отличному от адреса оригинального щупальца
                assert(&male_copy.GetTentacle(i) != &male.GetTentacle(i));
                // Каждое щупальце копии прицепляется к тому же щупальцу, что и оригинальное
                assert(male_copy.GetTentacle(i).GetLinkedTentacle() == male.GetTentacle(i).GetLinkedTentacle());
            }
        }
        // Если вы видите эту надпись, то разрушение осьминогов, скорее всего,
        // прошло без неопределённого поведения
        cout << "Everything is OK"s << endl;
    }
}
