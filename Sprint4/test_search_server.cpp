#include <vector>
#include <cmath>

#include "test_search_server.hpp"
#include "testing_framework.h"
#include "search_server.hpp"
#include "string_processing.hpp"

void TestStopWordsExclusion() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("in"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        
        ASSERT_EQUAL(found_docs[0].id, 42);
    }
    
    {
        SearchServer server("in the"s);
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("in"s);
        
        ASSERT_HINT(found_docs.empty(), "Stop words must be excluded from documents"s);
    }
}

void TestAddedDocumentsCanBeFound() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat in the city"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].id, 42);
    }
    
    {
        SearchServer server;
        
        server.AddDocument(42, "", DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat"s);
        
        ASSERT(found_docs.empty());
    }
}

void TestMinusWordsExcludeDocuments() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("-cat"s);
        
        ASSERT(found_docs.empty());
    }
    
    // minus words dont exclude documents where they dont appear in
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(43, "happy dog"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("-cat dog"s);
        
        ASSERT(found_docs.size() == 1);
    }
    
}

void TestMatchDocumentResults() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        const auto [words, status] = server.MatchDocument("fat cat out of city"s, 42);
        
        std::vector<std::string> desired_matched_words{"cat"s, "city"s};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::kActual);
    }
    
    // status kBanned
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(43, "happy dog"s, DocumentStatus::kBanned, ratings);
        
        const auto [words, status] = server.MatchDocument("fat cat out of city and a cute dog"s, 43);
        
        std::vector<std::string> desired_matched_words{"dog"s};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::kBanned);
    }
}

void TestFindTopDocumentsResultsSorting() {
    constexpr double kAccuracy = 1e-6;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(2, "dog city potato"s, DocumentStatus::kActual, ratings);
        server.AddDocument(3, "dog city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(4, "lorem ipsum"s, DocumentStatus::kActual, ratings);
        server.AddDocument(5, "city"s, DocumentStatus::kBanned, ratings); // kkBanned
        server.AddDocument(6, "frog city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(7, "the cat says meow to dog"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("dog in the city"s);
        
        ASSERT_EQUAL(found_docs.size(), 5u);
        
        ASSERT(std::is_sorted(found_docs.begin(), found_docs.end(), [](const Document& left, const Document& right) {
            if (std::abs(left.relevance - right.relevance) < kAccuracy) {
                return left.rating > right.rating;
            } else {
                return left.relevance > right.relevance;
            }
        }));
    }
    
    {
        SearchServer server;
        
        server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(2, "dog city potato"s, DocumentStatus::kActual, ratings);
        server.AddDocument(3, "dog city"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("cat loves NY city"s);
        
        ASSERT(found_docs.size() == 3);
        
        ASSERT(std::is_sorted(found_docs.begin(), found_docs.end(), [](const Document& left, const Document& right) {
            if (std::abs(left.relevance - right.relevance) < kAccuracy) {
                return left.rating > right.rating;
            } else {
                return left.relevance > right.relevance;
            }
        }));
    }
}

void TestRatingsCalculation() {
    const std::string content = "cat city"s;
    
    // positive ratings
    {
        SearchServer server;
        
        const std::vector<int> ratings = {1, 2, 3};
        
        server.AddDocument(1, content, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat loves NY city"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, 2);
    }
    
    // negative ratings
    {
        SearchServer server;
        
        const std::vector<int> ratings = {-1, -2, -3};
        
        (void) server.AddDocument(1, content, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat loves NY city"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, -2);
    }
}

void TestFilteringByPredicate() {
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    
    server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
    server.AddDocument(2, "dog city potato"s, DocumentStatus::kBanned, ratings);
    server.AddDocument(3, "dog city"s, DocumentStatus::kRemoved, ratings);
    
    // status predicate
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, [](int , DocumentStatus status, int ) {
            return status == DocumentStatus::kActual;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
    
    // rating predicate
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, [](int , DocumentStatus , int rating) {
            return rating == 2;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 3u); // only a doc with higher rating is found
        ASSERT_EQUAL(filtered_docs[0].rating, 2);
    }
    
    // id predicate
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, [](int document_id, DocumentStatus , int ) {
            return document_id == 1;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
}

void TestFilteringByStatus() {
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    
    server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
    server.AddDocument(2, "dog city potato"s, DocumentStatus::kBanned, ratings);
    
    // explicit status
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, DocumentStatus::kBanned);
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 2);
    }
    
    // default status
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s);
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
}

void TestRelevanceCalculation() {
    constexpr double kAccuracy = 1e-6;
    
    SearchServer server;
    
    server.AddDocument(0, "cat cat city dog"s, DocumentStatus::kActual, {1});
    server.AddDocument(1, "city dog"s, DocumentStatus::kActual, {1});
    server.AddDocument(2, "cat city potato"s, DocumentStatus::kActual, {1});
    
    // positive case
    {
        std::vector<Document> found_docs = server.FindTopDocuments("cat"s);
        
        // log(documents_count * 1.0 / a) * (b / c)
        double expected_relevance_doc_0 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (2.0 / 4.0);
        double expected_relevance_doc_2 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (1.0 / 3.0);
        
        ASSERT_EQUAL(found_docs.size(), 2u);
        
        ASSERT(std::abs(found_docs[0].relevance - expected_relevance_doc_0) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance - expected_relevance_doc_2) < kAccuracy);
    }
    
    // zero relevance
    {
        std::vector<Document> found_docs = server.FindTopDocuments("city"s);
        
        ASSERT_EQUAL(found_docs.size(), 3u);
        
        ASSERT(std::abs(found_docs[0].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[2].relevance) < kAccuracy);
    }
}

void TestSplitIntoWordsEscapesSpaces() {
    ASSERT_EQUAL((std::vector<std::string> {"hello"s, "bro"s}), string_processing::SplitIntoWords("   hello    bro    "s));
    ASSERT_EQUAL(std::vector<std::string>{}, string_processing::SplitIntoWords("                 "s));
}

void TestGetDocumentIdReturnsId() {
    SearchServer search_server;
    
    search_server.AddDocument(1, "???????????????? ?????? ???????????????? ??????????"s, DocumentStatus::kActual, {7, 2, 7});
    search_server.AddDocument(2, "?????????????? ??????"s, DocumentStatus::kActual, {7, 2, 7});
    
    ASSERT_EQUAL(search_server.GetDocumentId(0), 1);
    ASSERT_EQUAL(search_server.GetDocumentId(1), 2);
}

void TestGetDocumentIdThrowsOutOfRange() {
    SearchServer search_server;
    
    try {
        search_server.GetDocumentId(-1);
    } catch (std::out_of_range& e) {
        return;
    }
    
    ASSERT_HINT(false, "GetDocumentId with negative id throws out_of_range"s);
}

void TestAddDocumentWithRepeatingId() {
    SearchServer search_server;
    
    search_server.AddDocument(1, "???????????????? ?????? ???????????????? ??????????"s, DocumentStatus::kActual, {7, 2, 7});
    
    try {
        search_server.AddDocument(1, "???????????????? ?????? ?? ???????????? ??????????????"s, DocumentStatus::kActual, {1, 2});
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "adding document with repeating id is not handled"s);
}

void TestAddDocumentWithNegativeId() {
    SearchServer search_server;
    
    try {
        search_server.AddDocument(-1, "???????????????? ?????? ?? ???????????? ??????????????"s, DocumentStatus::kActual, {1, 2});
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "adding document with negative id is not handled"s);
}

void TestAddDocumentWithSpecialSymbol() {
    SearchServer search_server;
    
    try {
        search_server.AddDocument(1, "?????????????? ?????? ????????\x12??????"s, DocumentStatus::kActual, {1, 2});
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "adding document containing unaccaptable symbol is not handled"s);
    
}

void TestQueryWithSpecialSymbol() {
    SearchServer search_server;
    
    try {
        search_server.FindTopDocuments("?????????????? ?????? ????????\x12??????"s);
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "query with symbols in not handled"s);
}

void TestDoubleMinusWord() {
    SearchServer search_server;
    
    try {
        search_server.FindTopDocuments("--????????????????"s);
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "query with double minus in not handled"s);
}

void TestEmptyMinusWord() {
    SearchServer search_server;
    
    try {
        search_server.FindTopDocuments("???????????????? -"s);
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "query with empty minus word is not handled"s);
}

void TestSearchNonExistentWord() {
    SearchServer search_server;
    
    search_server.FindTopDocuments("potato");
}

void TestSearchServer() {
    RUN_TEST(TestStopWordsExclusion);
    RUN_TEST(TestAddedDocumentsCanBeFound);
    RUN_TEST(TestMinusWordsExcludeDocuments);
    RUN_TEST(TestMatchDocumentResults);
    RUN_TEST(TestFindTopDocumentsResultsSorting);
    RUN_TEST(TestRatingsCalculation);
    RUN_TEST(TestFilteringByPredicate);
    RUN_TEST(TestFilteringByStatus);
    RUN_TEST(TestRelevanceCalculation);
    
    // tests for Sprint3
    RUN_TEST(TestGetDocumentIdThrowsOutOfRange);
    RUN_TEST(TestSearchNonExistentWord);
    
    RUN_TEST(TestSplitIntoWordsEscapesSpaces);
    RUN_TEST(TestGetDocumentIdReturnsId);
    RUN_TEST(TestAddDocumentWithRepeatingId);
    RUN_TEST(TestAddDocumentWithNegativeId);
    RUN_TEST(TestAddDocumentWithSpecialSymbol);
    RUN_TEST(TestDoubleMinusWord);
    RUN_TEST(TestQueryWithSpecialSymbol);
    RUN_TEST(TestEmptyMinusWord);
}
