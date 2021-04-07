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
    vector<string> buses;
};

ostream& operator<<(ostream& os, const BusesForStopResponse& r) {
    if (r.buses.empty()) {
        os << "No stop"s << endl;
    } else {
        bool isFirst = true;
        for (const string& bus : r.buses) {
            // avoid extra space at the end of the output
            if(isFirst) {
                os << bus;
                isFirst = false;
                continue;
            }
            os << " "s << bus;
        }
    }
    return os;
}

struct StopsForBusResponse {
    string bus;
    vector<pair<string, vector<string>>> stops_to_buses;
};

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
    if (r.stops_to_buses.empty()) {
        os << "No bus"s;
    } else {
        bool isFirstLine = true;
        
        for (const auto& [stop, buses] : r.stops_to_buses) {
            if (isFirstLine) {
                isFirstLine = false;
            } else {
                os << endl;
            }
            
            os << "Stop "s << stop << ": "s;
            if (buses.size() == 1) {
                os << "no interchange"s;
            } else {
                bool isFirstBus = true;
                for (const string& other_bus : buses) {
                    if (r.bus != other_bus) {
                        if(isFirstBus) {
                            os << other_bus;
                            isFirstBus = false;
                            continue;
                        }
                        os << " "s << other_bus;
                    }
                }
            }
        }
    }
    
    return os;
}

struct AllBusesResponse {
    map<string, vector<string>> buses_to_stops;
};

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
    if (r.buses_to_stops.empty()) {
        os << "No buses"s;
    } else {
        bool isFirstLine = true;
        for (const auto& bus_item : r.buses_to_stops) {
            if (isFirstLine) {
                isFirstLine = false;
            } else {
                os << endl;
            }
            
            os << "Bus "s << bus_item.first << ": "s;
            bool isFirstStop = true;
            for (const string& stop : bus_item.second) {
                if(isFirstStop) {
                    os << stop;
                    isFirstStop = false;
                    continue;
                }
                os << " "s << stop ;
            }
        }
    }
    return os;
}

//class BusManager {
//public:
//    void AddBus(const string& bus, const vector<string>& stops) {
//        // Реализуйте этот метод
//    }
//
//    BusesForStopResponse GetBusesForStop(const string& stop) const {
//        // Реализуйте этот метод
//    }
//
//    StopsForBusResponse GetStopsForBus(const string& bus) const {
//        // Реализуйте этот метод
//    }
//
//    AllBusesResponse GetAllBuses() const {
//        // Реализуйте этот метод
//    }
//
//private:
//    map<string, vector<string>> buses_to_stops_, stops_to_buses_;
//};

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

void TestOutputBusesForStopNoStop() {
    BusesForStopResponse response;
    
    ostringstream output;
    
    output << response;
    
    assert(output.str() == "No stop"s);
}

void TestOutputBusesForStop() {
    BusesForStopResponse response;
    
    response.buses = {"32"s, "32K"s};
    
    ostringstream output;
    
    output << response;
    
    assert(output.str() == "32 32K"s);
}

void TestOutputStopsForBusEmpty() {
    StopsForBusResponse response;
    
    ostringstream output;
    
    output << response;
    
    assert(output.str() == "No bus"s);
}

void TestOutputStopsForBus() {
    StopsForBusResponse response;
    
    response.bus = "272"s;
    response.stops_to_buses = {{"Vnukovo", {"32"s, "32K"s, "950"s, "272"s}},
                               {"Moskovsky"s,{"272"s}},
                               {"Rumyantsevo"s,{"272"s}},
                               {"Troparyovo"s,{"950"s, "272"s}}
                              };
    
    ostringstream output;
    
    output << response;
    
//    cout << response;
    
    assert(output.str() == ("Stop Vnukovo: 32 32K 950\nStop Moskovsky: no interchange\nStop Rumyantsevo: no interchange\nStop Troparyovo: 950"s));
}

void TestOutputAllBuses() {
    AllBusesResponse response;
    
    response.buses_to_stops = {{"272"s, {"Vnukovo"s, "Moskovsky"s, "Rumyantsevo"s, "Troparyovo"s}},
                               {"32"s, {"Tolstopaltsevo"s, "Marushkino"s, "Vnukovo"s}},
                              };
    
    ostringstream output;
    output << response;
    
//    cout << "my output\n";
//    cout << output.str() << endl;
    
    ostringstream reference_output;
    reference_output << "Bus 272: Vnukovo Moskovsky Rumyantsevo Troparyovo"s << endl << "Bus 32: Tolstopaltsevo Marushkino Vnukovo"s;

//    cout << "reference output\n";
//    cout << reference_output.str() << endl;
//
//    cout << output.str().size();
//
//    cout << reference_output.str().size();
    
    assert(output.str() == reference_output.str());
}

static void RunTests() {
    TestQueryInputNewBus();
    TestQueryInputAllBuses();
    TestQueryInputStopsForBus();
    TestQueryInputBusesForStop();
    
    TestOutputBusesForStop();
    TestOutputStopsForBusEmpty();
    TestOutputStopsForBus();
    TestOutputAllBuses();
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
