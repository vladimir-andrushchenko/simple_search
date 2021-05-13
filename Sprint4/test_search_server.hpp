#ifndef test_search_server_hpp
#define test_search_server_hpp

void TestStopWordsExclusion();

void TestAddedDocumentsCanBeFound();

void TestMinusWordsExcludeDocuments();

void TestMatchDocumentResults();

void TestFindTopDocumentsResultsSorting();

void TestRatingsCalculation();

void TestFilteringByPredicate();

void TestFilteringByStatus();

void TestRelevanceCalculation();

void TestSplitIntoWordsEscapesSpaces();

void TestGetDocumentIdThrowsOutOfRange();

void TestAddDocumentWithRepeatingId();

void TestAddDocumentWithNegativeId();

void TestAddDocumentWithSpecialSymbol();

void TestQueryWithSpecialSymbol();

void TestDoubleMinusWord();

void TestEmptyMinusWord();

void TestSearchNonExistentWord();

void TestSearchServer();

#endif /* test_search_server_hpp */
