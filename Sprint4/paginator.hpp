#pragma once

#include <vector>
#include <string>
#include <algorithm>

using namespace std::literals;

template<typename InputIterator>
class IteratorRange {
public:
    IteratorRange(InputIterator range_begin, InputIterator range_end): begin_iterator_(range_begin), end_iterator_(range_end) {}
    
public:
    InputIterator begin() const {
        return begin_iterator_;
    }
    
    InputIterator end() const {
        return end_iterator_;
    }
    
    size_t size() const {
        return std::distance(begin_iterator_, end_iterator_);
    }
    
private:
    InputIterator begin_iterator_;
    InputIterator end_iterator_;
};

//void FillRandom(vector<int>& v, int n) {
//    v.reserve(n);
//
//    for (int i = 0; i < n; i += 15) {
//        int number = rand();
//
//        // мы можем заполнить 15 элементов вектора,
//        // но не более, чем нам осталось до конца:
//        int count = min(15, n - i);
//
//        for (int j = 0; j < count; ++j)
//            // таким образом, получим j-й бит числа.
//            // операцию побитового сдвига вы уже видели в этой программе
//            // на этот раз двигаем вправо, чтобы нужный бит оказался самым последним
//            v.push_back((number >> j) % 2);
//    }
//}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator range_begin, Iterator range_end, size_t page_size) {
        if (range_begin == range_end) {
            throw std::invalid_argument("no empty ranges"s);
        }
        
        size_t left = std::distance(range_begin, range_end);
        
        while (left > 0) {
            const auto current_page_size = std::min(page_size, left);
            pages_.push_back({range_begin, range_begin + current_page_size});
            left -= current_page_size;
            range_begin += current_page_size;
        }
    }
    
public:
    auto begin() const {
        return pages_.begin();
    }
    
    auto end() const {
        return pages_.end();
    }
    
    size_t size() {
        return pages_.size();
    }
    
private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(std::begin(c), std::end(c), page_size);
}

template<typename IteratorRangeType>
std::ostream& operator<<(std::ostream& output, const IteratorRange<IteratorRangeType>& iterator_range) {
    for (auto it = iterator_range.begin(); it != iterator_range.end(); ++it) {
        output << *it;
    }
    return output;
}
