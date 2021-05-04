#include <iostream>

using namespace std;

bool IsPowOfTwo(int n) {
    if (n == 1) return true;
    if (n % 2 != 0 || n == 0) return false;
    return IsPowOfTwo(n / 2);
}

int main() {
    int result = IsPowOfTwo(8);
    cout << result << endl;
}
