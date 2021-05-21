#include <cstdint>
#include <iostream>

using namespace std;

// упростите эту экспоненциальную функцию,
// реализовав линейный алгоритм
int64_t T(int i) {
    if (i <= 1) {
        return 0;
    }
    if (i == 2) {
        return 1;
    }
    
    int first = 0, second = 0;
    int third = 1;

    for (int t = 3; t <= i; ++t) {
        int curr = first + second + third;
        first = second;
        second = third;
        third = curr;
    }
    
    return third;
}

int main() {
    int i;

    while (true) {
        cout << "Enter index: "s;
        if (!(cin >> i)) {
            break;
        }

        cout << "Ti = "s << T(i) << endl;
    }
}
