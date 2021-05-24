#include <string>
#include <iostream>

#include "document.hpp"

using namespace std::literals;

Document::Document() : id(0), relevance(0), rating(0) {}

Document::Document(int id, double relevance, int rating): id(id), relevance(relevance), rating(rating) {}

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

std::ostream& operator<<(std::ostream& out, const DocumentStatus status) {
    switch (status) {
        case DocumentStatus::kActual:
            out << "kActual"s;
            break;
        case DocumentStatus::kBanned:
            out << "kBanned"s;
            break;
        case DocumentStatus::kIrrelevant:
            out << "kIrrelevant"s;
            break;
        case DocumentStatus::kRemoved:
            out << "kRemoved"s;
            break;
    }
    
    return out;
}
