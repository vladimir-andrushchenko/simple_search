#include <iostream>
#include <algorithm>
#include <set>
#include "Test.h"

using namespace std;

//bool IsPowOfTwo(int n) {
//    if (n == 1) return true;
//    if (n % 2 != 0 || n == 0) return false;
//    return IsPowOfTwo(n / 2);
//}

//void PrintSpacesPositions(string& str) {
//    constexpr char space = ' ';
//    for (auto it = std::find(str.begin(), str.end(), space);
//         it != str.end();
//         it = find(std::next(it), str.end(), space)) {
//        std::cout << std::distance(str.begin(), it) << std::endl;
//    }
//}

//set<int>::const_iterator FindNearestElement(const set<int>& numbers, int border) {
//    if (numbers.empty()) {
//        return numbers.end();
//    }
//
//    auto l_bound = numbers.lower_bound(border);
//
//    if (l_bound != numbers.end() && *l_bound == border) {
//        return l_bound;
//    }
//
//    if (l_bound != numbers.begin()) {
//        std::advance(l_bound, -1);
//    }
//
//    const auto u_bound = numbers.upper_bound(border);
//
//    if (u_bound == numbers.end()) {
//        return l_bound;
//    }
//
//    if (std::abs(*l_bound - border) > std::abs(*u_bound - border)) {
//        return u_bound;
//    }
//
//    return l_bound;
//}

//int main() {
//    set<int> numbers = {1, 4, 6};
//    cout << *FindNearestElement(numbers, 0) << " " << *FindNearestElement(numbers, 3) << " "
//         << *FindNearestElement(numbers, 5) << " " << *FindNearestElement(numbers, 6) << " "
//         << *FindNearestElement(numbers, 100) << endl;
//
//    set<int> empty_set;
//
//    cout << (FindNearestElement(empty_set, 8) == end(empty_set)) << endl;
//    return 0;
//}

set<int>::const_iterator FindNearestElement(const set<int>& numbers, int border) {
    // Запрашиваем итератор на первый элемент, не меньший border;
    // если такого элемента нет, то мы получим numbers.end()
    const auto first_not_less = numbers.lower_bound(border);

    // Если множество пусто или до первого элемента не меньше border
    // нет элементов, то мы уже получили ответ
    if (first_not_less == numbers.begin()) {
        return first_not_less;
    }

    // Если элементов, не меньших border, нет и set не пуст, то достаточно взять
    // итератор на последний элемент, меньший border
    // prev -> http://ru.cppreference.com/w/cpp/iterator/prev
    const auto last_less = prev(first_not_less);
    if (first_not_less == numbers.end()) {
        return last_less;
    }

    // Разыменуем оба итератора-кандидата и выберем тот,
    // чей элемент ближе к искомому
    const bool is_left = (border - *last_less <= *first_not_less - border);
    return is_left ? last_less : first_not_less;
}

//template <typename RandomIt>
//pair<RandomIt, RandomIt> FindStartsWith(RandomIt range_begin, RandomIt range_end, const std::string& prefix) {
//auto lb = std::lower_bound(range_begin, range_end, prefix);
//
//    // For the upper bound we want to view the vector's data as if
//    // every element was truncated to the size of the prefix.
//    // Then perform a normal match.
//    auto ub = std::upper_bound(lb, range_end, prefix,
//    [&](std::string const& s1, std::string const& s2)
//    {
//        // compare UP TO the length of the prefix and no farther
//        if(auto cmp = std::strncmp(s1.data(), s2.data(), prefix.size()))
//            return cmp < 0;
//
//        // The strings are equal to the length of the prefix so
//        // behave as if they are equal. That means s1 < s2 == false
//        return false;
//    });
//
//    // make the answer look like we used std::equal_range
//    // (if that's what's needed)
//    return std::make_pair(lb, ub);
//}

template <typename RandomIt>
pair<RandomIt, RandomIt> FindStartsWith(RandomIt range_begin, RandomIt range_end, string prefix) {
    // Все строки, начинающиеся с prefix, больше или равны строке "<prefix>"
    auto left = lower_bound(range_begin, range_end, prefix);
    // Составим строку, которая в рамках буквенных строк является точной верхней гранью
    // множества строк, начинающихся с prefix
    string upper_bound = prefix;
    
    ++upper_bound[upper_bound.size() - 1];
    // Первое встреченное слово, не меньшее upper_bound, обязательно является концом полуинтервала
    auto right = lower_bound(range_begin, range_end, upper_bound);
    return {left, right};
}

string IncrementLetters(string text) {
    for (char& letter : text) {
        ++letter;
    }
    
    return text;
}

int main() {
    
    cout << IncrementLetters("abc") << endl;
//    const vector<string> sorted_strings = {"moscow", "motovilikha", "murmansk"};
//
//    const auto mo_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), "mo");
//    for (auto it = mo_result.first; it != mo_result.second; ++it) {
//        cout << *it << " ";
//    }
//    cout << endl;
//
//    const auto mt_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), "mt");
//    cout << (mt_result.first - begin(sorted_strings)) << " " << (mt_result.second - begin(sorted_strings)) << endl;
//
//    const auto na_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), "na");
//    cout << (na_result.first - begin(sorted_strings)) << " " << (na_result.second - begin(sorted_strings)) << endl;

    return 0;
}
