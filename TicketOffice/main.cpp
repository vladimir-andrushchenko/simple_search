#include <algorithm>
#include <deque>
#include <string>

using namespace std;

struct Ticket {
    int id;
    string name;
};

class TicketOffice {
public:
    // добавить билет в систему
    void PushTicket(const string& name) {
        // реализуйте метод
    }

    // получить количество доступных билетов
    int GetAvailable() const {
        // реализуйте метод
    }

    // получить количество доступных билетов определённого типа
    int GetAvailable(const string& name) const {
        // реализуйте метод
    }

    // отозвать старые билеты (до определённого id)
    void Invalidate(int minimum) {
        // реализуйте метод
    }

private:
    int last_id_ = 0;
    deque<Ticket> tickets_;
};
