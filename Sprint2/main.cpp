#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

std::string ReadLine() {
    std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            words.push_back(word);
            word = "";
        }
        else {
            word += c;
        }
    }
    words.push_back(word);
    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

/* Ïîäñòàâüòå âàøó ðåàëèçàöèþ êëàññà SearchServer ñþäà */

class SearchServer {
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;

public:
    void SetStopWords(const std::string& text) {
        for (const std::string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const std::string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id,
            DocumentData{
                ComputeAverageRating(ratings),
                status
            });
    }
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const{
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](const int document_id, const DocumentStatus ds, const int rating) { return status == ds; });
    }
    template<typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);

        std::sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        std::vector<std::string> matched_words;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return { matched_words, documents_.at(document_id).status };
    }

private:

    bool IsStopWord(const std::string& word) const {
        return stop_words_.count(word) > 0;
    }

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const {
        std::vector<std::string> words;
        for (const std::string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const std::vector<int>& ratings) {
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {
            text,
            is_minus,
            IsStopWord(text)
        };
    }

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const {
        Query query;
        for (const std::string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template<typename Filter>
    std::vector<Document> FindAllDocuments(const Query& query, const Filter& filter) const {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }

        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance) {
            if (filter(document_id, documents_.at(document_id).status, documents_.at(document_id).rating))
                matched_documents.push_back({
                    document_id,
                    relevance,
                    documents_.at(document_id).rating
                    });
        }
        return matched_documents;
    }
};
// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        assert(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        assert(doc0.id == doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        assert(server.FindTopDocuments("in"s).empty());
    }
}

/*
Разместите код остальных тестов здесь
*/

// Добавление документов. Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void TestAddDocument() {
    { // test word
        SearchServer server;
        
        server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
        
        const auto& found_docs = server.FindTopDocuments("cat"s);
        
        assert(found_docs.size() == 1);
        assert(found_docs[0].id == 0);
        
    }
    
    { // test empty search server
        SearchServer server;
        
        const auto& found_docs = server.FindTopDocuments("cat"s);
        
        assert(found_docs.empty());
        
    }
}

// Поддержка стоп-слов. Стоп-слова исключаются из текста документов
void TestStopWords() {
    SearchServer server;
    
    server.SetStopWords("stop");
    
    server.AddDocument(0, "word stop"s, DocumentStatus::ACTUAL, {1, 2, 3});
    
    const auto& found_docs = server.FindTopDocuments("word"s);
    
    assert(found_docs[0].id == 0);
    
    const auto& found_docs_after_search_with_only_stops_word = server.FindTopDocuments("stop"s);
    
    assert(found_docs_after_search_with_only_stops_word.empty());
}

//  Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestMinusWords() {
    {
        SearchServer server;
        server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
        server.AddDocument(1, "happy cat"s, DocumentStatus::ACTUAL, {8, -3});
        const auto found_docs = server.FindTopDocuments("-cat"s);
        assert(found_docs.size() == 0);
    }
    {
        SearchServer server;
        server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
        server.AddDocument(1, "happy cat"s, DocumentStatus::ACTUAL, {8, -3});
        const auto& found_docs = server.FindTopDocuments("-happy cat"s);
        assert(found_docs.size() == 1);
        assert(found_docs[0].id == 0);
    }
}

//Матчинг документов. При матчинге документа по поисковому запросу должны быть возвращены все слова из поискового запроса, присутствующие в документе. Если есть соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
void TestMatchDocument() {
    SearchServer server;
    server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(1, "happy cat"s, DocumentStatus::BANNED, {8, -3});
    
    { // all ok
        const auto [words, status] = server.MatchDocument("cat the potato", 0);
        
        std::vector<std::string> desired_matched_words{"cat"s, "the"s};
        assert(words == desired_matched_words);
        assert(status == DocumentStatus::ACTUAL);
    }
    
    { // all ok
        const auto [words, status] = server.MatchDocument("cat the potato", 1);
        
        std::vector<std::string> desired_matched_words{"cat"s};
        assert(words == desired_matched_words);
        assert(status == DocumentStatus::BANNED);
    }
    
    { // empty match string
        const auto [words, status] = server.MatchDocument("", 0);
        
        assert(words.empty());
    }
    
    
    { // minus word
        const auto& [words, status] = server.MatchDocument("cat -the potato", 0);
        
        assert(words.empty());
    }
    
    { // minus word misses
        const auto [words, status] = server.MatchDocument("cat the -potato", 0);
        
        std::vector<std::string> desired_matched_words{"cat"s, "the"s};
        assert(words == desired_matched_words);
    }
    
    { // minus word misses
        const auto [words, status] = server.MatchDocument("-potato", 0);
        
        assert(words.empty());
    }
}

// Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestRelevanceSort() {
    SearchServer server;
    server.SetStopWords("и"s);

    
    server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(1, "happy cat"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(2, "sad potato"s, DocumentStatus::ACTUAL, {10000});

    const auto found_docs = server.FindTopDocuments("cat potato"s);
    assert(found_docs.size() == 3);
    
    assert(found_docs[0].relevance > found_docs[1].relevance);
    assert(found_docs[0].relevance > found_docs[2].relevance);
    assert(found_docs[1].relevance > found_docs[2].relevance);
}

// Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestRating() {
    SearchServer server;

    server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(1, "happy cat"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(2, "sad potato"s, DocumentStatus::ACTUAL, {10000});
    server.AddDocument(3, "stupid cute cat in the NY"s, DocumentStatus::ACTUAL, {2147483647, 1});

    const auto found_docs = server.FindTopDocuments("cat potato"s);
    
    assert(found_docs[0].rating == 10000);
    assert(found_docs[1].rating == 1);
    assert(found_docs[2].rating == 2);
    // 2147483647
    
   
}

// Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestPredicateFiltering() {
    SearchServer server;

    server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(1, "happy cat"s, DocumentStatus::BANNED, {2});
    server.AddDocument(2, "happy cat"s, DocumentStatus::REMOVED, {1});
    server.AddDocument(3, "happy cat"s, DocumentStatus::IRRELEVANT, {1});

    
    {
        const auto find_documents_with_actual_status = []([[maybe_unused]] int document_id, DocumentStatus status,
                                                          [[maybe_unused]] int rating) {
            return status == DocumentStatus::ACTUAL;
        };
                                    
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                                   find_documents_with_actual_status);
        
        assert(filtered_docs.size() == 1);
        assert(filtered_docs[0].id == 0);
    }
    
    {
        const auto find_documents_with_banned_status = []([[maybe_unused]] int document_id, DocumentStatus status,
                                                          [[maybe_unused]] int rating) {
            return status == DocumentStatus::BANNED;
        };
                                    
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                            find_documents_with_banned_status);
        
        assert(filtered_docs.size() == 1);
        assert(filtered_docs[0].id == 1);
    }
    
    {
        const auto find_documents_with_removed_status = []([[maybe_unused]] int document_id, DocumentStatus status,
                                                          [[maybe_unused]] int rating) {
            return status == DocumentStatus::REMOVED;
        };
                                    
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                            find_documents_with_removed_status);
        
        assert(filtered_docs.size() == 1);
        assert(filtered_docs[0].id == 2);
    }
    
    {
        const auto find_documents_with_irrelevant_status = []([[maybe_unused]] int document_id, DocumentStatus status,
                                                          [[maybe_unused]] int rating) {
            return status == DocumentStatus::IRRELEVANT;
        };
                                    
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                            find_documents_with_irrelevant_status);
        
        assert(filtered_docs.size() == 1);
        assert(filtered_docs[0].id == 3);
    }
    
    {
        const auto find_documents_with_rating = []([[maybe_unused]] int document_id,
                                                          [[maybe_unused]] DocumentStatus status,
                                                          [[maybe_unused]] int rating) {
            return rating > 1;
        };
                                    
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                            find_documents_with_rating);
        
        assert(filtered_docs.size() == 2);
    }
    
    {
        const auto find_documents_with_id = []([[maybe_unused]] int document_id,
                                                          [[maybe_unused]] DocumentStatus status,
                                                          [[maybe_unused]] int rating) {
            return document_id == 1;
        };
                                    
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                            find_documents_with_id);
        
        assert(filtered_docs.size() == 1);
    }
    
    {
        const auto find_documents_with_id = []([[maybe_unused]] int document_id,
                                                          [[maybe_unused]] DocumentStatus status,
                                                          [[maybe_unused]] int rating) {
            return document_id == -1;
        };
                                    
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                            find_documents_with_id);
        
        assert(filtered_docs.size() == 0);
    }
}

// Поиск документов, имеющих заданный статус.
void TestFilteringWithStatus() {
    SearchServer server;

    server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(1, "happy cat"s, DocumentStatus::BANNED, {2});
    
    {
        const auto& filtered_docs = server.FindTopDocuments("cat"s,
                                                            DocumentStatus::BANNED);
        
        assert(filtered_docs.size() == 1);
    }
    
    const auto& filtered_docs = server.FindTopDocuments("cat"s);
    
    assert(filtered_docs.size() == 1);
}

//  Корректное вычисление релевантности найденных документов.
void TestRelevance() {
    SearchServer server;
    server.SetStopWords("и"s);
    /*
     белый кот и модный ошейник
     пушистый кот пушистый хвост
     ухоженный пёс выразительные глаза
     */

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {1});

    const auto& found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
    assert(found_docs.size() == 3);
//    relevance    double    0.27465307216702745
    
    assert(found_docs[0].relevance == 0.65067242136109593);
    assert(found_docs[1].relevance == 0.27465307216702745);
    
//    assert((found_docs[0].relevance > 0.65) &&
//           (found_docs[0].relevance < 0.66));
//
//    assert((found_docs[1].relevance > 0.27) &&
//           (found_docs[1].relevance < 0.28));
//
//    assert((found_docs[2].relevance > 0.10) &&
//           (found_docs[2].relevance < 0.11));
}



// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    // Не забудьте вызывать остальные тесты здесь
    TestAddDocument();
    TestStopWords();
    TestMinusWords();
    TestMatchDocument();
    TestRelevanceSort();
    TestRating();
    TestPredicateFiltering();
    TestFilteringWithStatus();
    TestRelevance();
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    std::cout << "Search server testing finished"s << std::endl;
}

//void PrintDocument(const Document& document) {
//    std::cout << "{ "s
//        << "document_id = "s << document.id << ", "s
//        << "relevance = "s << document.relevance << ", "s
//        << "rating = "s << document.rating
//        << " }"s << std::endl;
//}
//
//int main() {
//    SearchServer search_server;
//    search_server.SetStopWords("и в на"s);
//
//    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
//    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
//    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
//    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
//
//    std::cout << "ACTUAL by default:"s << std::endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
//        PrintDocument(document);
//    }
//
//    const auto find_documents_with_actual_status = []([[maybe_unused]] int document_id, DocumentStatus status,
//                                                      [[maybe_unused]] int relevance) {
//            return status == DocumentStatus::ACTUAL;
//        };
//
//    std::cout << "ACTUAL:"s << std::endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
//                                                                   find_documents_with_actual_status)) {
//        PrintDocument(document);
//    }
//
//    std::cout << "BANNED:"s << std::endl;
//        for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
//                                                                       DocumentStatus::BANNED)) {
//            PrintDocument(document);
//        }
//
//    const auto find_documents_with_even_id_numbers = [](int document_id, [[maybe_unused]] DocumentStatus status,
//                                                        [[maybe_unused]] int relevance) {
//        return document_id % 2 == 0;
//    };
//
//    std::cout << "Even ids:"s << std::endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
//                                                                   find_documents_with_even_id_numbers)) {
//        PrintDocument(document);
//    }
//
//    return 0;
//}
