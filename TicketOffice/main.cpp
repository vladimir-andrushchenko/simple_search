#include <algorithm>
#include <deque>
#include <string>
#include <cassert>

using namespace std;

struct Ticket {
    int id;
    string name;
    
    Ticket(int given_id, string given_name): id(given_id), name(given_name) {}
};

class TicketOffice {
public:
    // добавить билет в систему
    void PushTicket(const string& name) {
        tickets_.push_back(Ticket(last_id_, name));
        ++last_id_;
    }

    // получить количество доступных билетов
    int GetAvailable() const {
        return static_cast<int>(tickets_.size());
    }

    // получить количество доступных билетов определённого типа
    int GetAvailable(const string& name) const {
        return static_cast<int>(count_if(tickets_.begin(), tickets_.end(), [name](const Ticket& ticket){
            return ticket.name == name;
        }));
    }

    // отозвать старые билеты (до определённого id)
    void Invalidate(int minimum) {
        const auto it = lower_bound(tickets_.begin(), tickets_.end(), minimum, [](const Ticket& ticket, int min){
            return ticket.id < min;
        });
        
        tickets_ = deque<Ticket>(it, tickets_.end());
    }

private:
    int last_id_ = 0;
    deque<Ticket> tickets_;
};

int main() {
    TicketOffice ticket_office;
    
    ticket_office.PushTicket("zero_ticket");
    ticket_office.PushTicket("first_ticket");
    ticket_office.PushTicket("second_ticket");
    
    assert(ticket_office.GetAvailable() == 3);
    assert(ticket_office.GetAvailable("zero_ticket"s) == 1);
    
    ticket_office.Invalidate(1);
    
    assert(ticket_office.GetAvailable() == 2);
}
