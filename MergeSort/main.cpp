#include <iostream>
#include <stdexcept>
#include <vector>
#include <numeric>

using namespace std;

template <typename It>
void PrintRange(It range_begin, It range_end) {
    for (auto it = range_begin; it != range_end; ++it) {
        cout << *it << " "s;
    }
    cout << endl;
}

template <typename RandomIt>
void Merge(RandomIt range_begin, RandomIt range_mid, RandomIt range_end) {
    auto runner = range_mid;
    while (range_begin != range_mid && runner != range_end) {
        if (*range_begin > *runner) {
            iter_swap(range_begin, runner);
            ++range_begin;
        } else {
            ++runner;
        }
    }
}

template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    if (distance(range_begin, range_end) == 1) {
        return;
    }
    
    auto range_mid = range_begin + (std::distance(range_begin, range_end) / 2);
    MergeSort(range_begin, range_mid);
    MergeSort(range_mid, range_end);
    inplace_merge(range_begin, range_mid, range_end);
}


int main()
{
    std::vector<int> v{9, 2, 10};
    MergeSort(v.begin(), v.end());
    for(auto n : v) {
        std::cout << n << ' ';
    }
    std::cout << '\n';
}
