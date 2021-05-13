#include <string>
#include <iostream>

#include "document.hpp"

using namespace std::literals;

void PrintDocument(const Document& document) {
    std::cout << "{ "s
    << "document_id = "s << document.id << ", "s
    << "relevance = "s << document.relevance << ", "s
    << "rating = "s << document.rating << " }"s;
}

std::ostream& operator<<(std::ostream& output, const Document& document) {
    PrintDocument(document);
    return output;
}
