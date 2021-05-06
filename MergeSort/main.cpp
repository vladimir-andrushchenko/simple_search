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
    auto range_length = range_end - range_begin;
    if (range_length < 2) {
        return;
    }
    
    vector<typename RandomIt::value_type> temp_range(range_begin, range_end);
    
    auto range_mid = temp_range.begin() + (range_length / 2);
    
    MergeSort(temp_range.begin(), range_mid);
    MergeSort(range_mid, temp_range.end());
    
    merge(temp_range.begin(), range_mid, range_mid, temp_range.end(), range_begin);
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
