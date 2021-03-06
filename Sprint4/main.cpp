#include <string>
#include <iostream>

#include "search_server.hpp"
#include "request_queue.hpp"
#include "test_search_server.hpp"
#include "paginator.hpp"

using namespace std::literals;

// https://onlinegdb.com/HkZIJT5du sometime exceedes compile time
int main() {
    TestSearchServer();
    std::cout << std::endl;

    std::cout << "Basic functionality of adding, finding, and matching documents"s << std::endl;

    {
        SearchServer search_server = CreateSearchServer("и в на"s);

        AddDocument(search_server, 1, "пушистый кот пушистый хвост"s, DocumentStatus::kActual, {7, 2, 7});
        AddDocument(search_server, 1, "пушистый пёс и модный ошейник"s, DocumentStatus::kActual, {1, 2});
        AddDocument(search_server, -1, "пушистый пёс и модный ошейник"s, DocumentStatus::kActual, {1, 2});
        AddDocument(search_server, 3, "большой пёс скво\x12рец евгений"s, DocumentStatus::kActual, {1, 3, 2});
        AddDocument(search_server, 4, "большой пёс скворец евгений"s, DocumentStatus::kActual, {1, 1, 1});

        FindTopDocuments(search_server, "пушистый -пёс"s);
        FindTopDocuments(search_server, "пушистый --кот"s);
        FindTopDocuments(search_server, "пушистый -"s);
        FindTopDocuments(search_server, "скво\x12рец"s);

        MatchDocuments(search_server, "пушистый пёс"s);
        MatchDocuments(search_server, "модный -кот"s);
        MatchDocuments(search_server, "модный --пёс"s);
        MatchDocuments(search_server, "пушистый - хвост"s);
    }

    std::cout << std::endl;

    std::cout << "Basic functionality of RequestQueue"s << std::endl;

    {
        SearchServer search_server = CreateSearchServer("and in at"s);
        RequestQueue request_queue(search_server);

        AddDocument(search_server, 1, "curly cat curly tail"s, DocumentStatus::kActual, {7, 2, 7});
        AddDocument(search_server, 2, "curly dog and fancy collar"s, DocumentStatus::kActual, {1, 2, 3});
        AddDocument(search_server, 3, "big cat fancy collar "s, DocumentStatus::kActual, {1, 2, 8});
        AddDocument(search_server, 4, "big dog sparrow Eugene"s, DocumentStatus::kActual, {1, 3, 2});
        AddDocument(search_server, 5, "big dog sparrow Vasiliy"s, DocumentStatus::kActual, {1, 1, 1});

        // 1439 запросов с нулевым результатом
        for (int i = 0; i < 1439; ++i) {
            request_queue.AddFindRequest("empty request"s);
        }
        // все еще 1439 запросов с нулевым результатом
        request_queue.AddFindRequest("curly dog"s);
        // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
        request_queue.AddFindRequest("big collar"s);
        // первый запрос удален, 1437 запросов с нулевым результатом
        request_queue.AddFindRequest("sparrow"s);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }
    
    std::cout << std::endl;

    std::cout << "Basic functionality of Paginator"s << std::endl;
    
    {
        SearchServer search_server("and with"s);

        search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::kActual, {7, 2, 7});
        search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::kActual, {1, 2, 3});
        search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::kActual, {1, 2, 8});
        search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::kActual, {1, 3, 2});
        search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::kActual, {1, 1, 1});

        const auto search_results = search_server.FindTopDocuments("curly dog"s);
        int page_size = 2;
        const auto pages = Paginate(search_results, page_size);

        // Выводим найденные документы по страницам
        for (auto page = pages.begin(); page != pages.end(); ++page) {
            std::cout << *page << std::endl;
            std::cout << "Page break"s << std::endl;
        }
    }
}

