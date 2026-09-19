#include <gtest/gtest.h>

#include "apigen/analyzer/api_analyzer.hpp"
#include "apigen/parser/har_parser.hpp"

class AnalyzerTest: public ::testing::Test
{
protected:
    apigen::APIAnalyzer analyzer;
    apigen::HarParser parser;
};

TEST_F(AnalyzerTest, PassDocumentCoding)
{
    const auto document = apigen::HttpDocument();
    EXPECT_NO_THROW(analyzer.analyze(document));
}
