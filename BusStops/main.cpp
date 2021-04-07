#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>


using namespace std;
using namespace std::literals;

enum class QueryType {
    NewBus,
    BusesForStop,
    StopsForBus,
    AllBuses,
};

struct Query {
    QueryType type;
    string bus;
    string stop;
    vector<string> stops;
};

istream& operator>>(istream& is, Query& q) {
    string operation_code;
    is >> operation_code;

    if (operation_code == "NEW_BUS"s) {
        q.type = QueryType::NewBus;
        
        is >> q.bus;
         
        int stop_count;
        
        is >> stop_count;

        q.stops.resize(stop_count);
        
        for (string& stop : q.stops) {
            is >> stop;
        }
    } else if (operation_code == "BUSES_FOR_STOP"s) {
        q.type = QueryType::BusesForStop;
        
        is >> q.stop;
    } else if (operation_code == "STOPS_FOR_BUS"s) {
        q.type = QueryType::StopsForBus;
        
        is >> q.bus;
    } else if (operation_code == "ALL_BUSES"s) {
        q.type = QueryType::AllBuses;
    }
    
    return is;
}

struct BusesForStopResponse {
    // Наполните полями эту структуру
};

ostream& operator<<(ostream& os, const BusesForStopResponse& r) {
    // Реализуйте эту функцию
    return os;
}

struct StopsForBusResponse {
    // Наполните полями эту структуру
};

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
    // Реализуйте эту функцию
    return os;
}

struct AllBusesResponse {
    // Наполните полями эту структуру
};

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
    // Реализуйте эту функцию
    return os;
}

class BusManager {
public:
    void AddBus(const string& bus, const vector<string>& stops) {
        // Реализуйте этот метод
    }

    BusesForStopResponse GetBusesForStop(const string& stop) const {
        // Реализуйте этот метод
    }

    StopsForBusResponse GetStopsForBus(const string& bus) const {
        // Реализуйте этот метод
    }

    AllBusesResponse GetAllBuses() const {
        // Реализуйте этот метод
    }

private:
    map<string, vector<string>> buses_to_stops_, stops_to_buses_;
};

void TestQueryInputAllBuses() {
    istringstream input;
    
    input.str("ALL_BUSES");
    
    Query query_all_buses;

    input >> query_all_buses;
    
    assert(query_all_buses.type == QueryType::AllBuses);
    
    assert(query_all_buses.stop == ""s);
    assert(query_all_buses.bus == ""s);
    assert(query_all_buses.stops.empty() == true);
}

void TestQueryInputNewBus() {
    istringstream input;
    
    input.str("NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s);
    
    Query query_new_bus;
    
    input >> query_new_bus;
    
    assert(query_new_bus.type == QueryType::NewBus);
    assert(query_new_bus.bus == "32"s);
    
    vector<string> bus_stops{"Tolstopaltsevo"s, "Marushkino"s, "Vnukovo"s};
    assert(query_new_bus.stops == bus_stops);
    
    assert(query_new_bus.stop == ""s);
}

void TestQueryInputStopsForBus() {
    istringstream input;
    
    input.str("STOPS_FOR_BUS 272"s);
    
    Query query_stops_for_bus;
    
    input >> query_stops_for_bus;
    
    assert(query_stops_for_bus.type == QueryType::StopsForBus);
    assert(query_stops_for_bus.bus == "272"s);
    assert(query_stops_for_bus.stop == ""s);
    assert(query_stops_for_bus.stops.empty() == true);
}

void TestQueryInputBusesForStop() {
    istringstream input;
    
    input.str("BUSES_FOR_STOP Vnukovo"s);
    
    Query query_buses_for_stop;
    
    input >> query_buses_for_stop;
    
    assert(query_buses_for_stop.type == QueryType::BusesForStop);
    assert(query_buses_for_stop.stop == "Vnukovo");
    assert(query_buses_for_stop.bus == ""s);
    assert(query_buses_for_stop.stops.empty() == true);
}

static void RunTests() {
    TestQueryInputNewBus();
    TestQueryInputAllBuses();
    TestQueryInputStopsForBus();
    TestQueryInputBusesForStop();
    cout << "all tests finished good" << endl;
}

// Не меняя тела функции main, реализуйте функции и классы выше

int main() {
    RunTests();
}


// original main

//int main() {
//    int query_count;
//    Query q;
//
//    cin >> query_count;
//
//    BusManager bm;
//    for (int i = 0; i < query_count; ++i) {
//        cin >> q;
//        switch (q.type) {
//            case QueryType::NewBus:
//                bm.AddBus(q.bus, q.stops);
//                break;
//            case QueryType::BusesForStop:
//                cout << bm.GetBusesForStop(q.stop) << endl;
//                break;
//            case QueryType::StopsForBus:
//                cout << bm.GetStopsForBus(q.bus) << endl;
//                break;
//            case QueryType::AllBuses:
//                cout << bm.GetAllBuses() << endl;
//                break;
//        }
//    }
//}
