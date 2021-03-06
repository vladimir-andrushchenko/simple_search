#include <string>
#include <iostream>

#include "search_server.hpp"
#include "request_queue.hpp"
#include "paginator.hpp"
#include "remove_duplicates.hpp"

#include "test_search_server.hpp"

using namespace std::literals;

int main() {
    TestSearchServer();
    
    std::cout << std::endl;
    
    SearchServer search_server("and with"s);

    search_server_helpers::AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::kActual, {7, 2, 7});
    search_server_helpers::AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::kActual, {1, 2});

    // дубликат документа 2, будет удалён
    search_server_helpers::AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::kActual, {1, 2});

    // отличие только в стоп-словах, считаем дубликатом
    search_server_helpers::AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::kActual, {1, 2});

    // множество слов такое же, считаем дубликатом документа 1
    search_server_helpers::AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::kActual, {1, 2});

    // добавились новые слова, дубликатом не является
    search_server_helpers::AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::kActual, {1, 2});

    // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
    search_server_helpers::AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::kActual, {1, 2});

    // есть не все слова, не является дубликатом
    search_server_helpers::AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::kActual, {1, 2});

    // слова из разных документов, не является дубликатом
    search_server_helpers::AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::kActual, {1, 2});
    
    std::cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
    remove_duplicates::RemoveDuplicates(search_server);
    std::cout << "After duplsicates removed: "s << search_server.GetDocumentCount() << std::endl;
}

