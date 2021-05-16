#pragma once

#include <vector>
#include <string>

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

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator range_begin, Iterator range_end, size_t page_size) {
        if (range_begin == range_end) {
            throw std::invalid_argument("no empty ranges"s);
        }
        
        for (auto it = range_begin; it != range_end;) {
            bool reached_end = false;
            
            for (size_t j = 1; j < page_size; ++j) {
                if (it + j == range_end) {
                    pages_.push_back({it, it + j});
                    reached_end = true;
                    break;
                }
            }
            
            if (reached_end) {
                break;
            }
            
            pages_.push_back({it, it + page_size});
            it += page_size;
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
