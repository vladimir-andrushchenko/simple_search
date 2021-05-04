#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

using namespace std;

template <typename It>
void PrintRange(It range_begin, It range_end) {
    for (auto it = range_begin; it != range_end; ++it) {
        cout << *it << " "s;
    }
    cout << endl;
}

template <typename Container, typename Element>
void FindAndPrint(const Container& container, const Element& element_to_find) {
    auto it_begin = begin(container);
    auto it_end = end(container);
    auto it = find(it_begin, it_end, element_to_find);
    PrintRange(it_begin, it);
    PrintRange(it, it_end);
}

//template <typename Container, typename Iterator>
//void EraseAndPrint(Container& container, Iterator it) {
//    const auto new_it = container.erase(it);
//    PrintRange(container.begin(), new_it);
//    PrintRange(new_it, container.end());
//}

//template <typename Container>
//void EraseAndPrint(Container& container, const int position) {
//    const auto new_it = container.erase(container.begin() + position);
//    PrintRange(container.begin(), new_it);
//    PrintRange(new_it, container.end());
//}

template <typename Container>
void EraseAndPrint(Container& container, const int position, const int start_index, const int end_index) {
    container.erase(container.begin() + position);
    PrintRange(container.begin(), container.end());
//    PrintRange(new_it, container.end());
    container.erase(container.begin() + start_index, container.begin() + end_index);
    PrintRange(container.begin(), container.end());
}

int main() {
    vector<string> langs = {"Python"s, "Java"s, "C#"s, "Ruby"s, "C++"s};
    EraseAndPrint(langs, 0, 0, 2);
}
