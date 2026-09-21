#include <gtest/gtest.h>


#include "apigen/analyzer/url_parser.hpp"
#include "apigen/core/error.hpp"

class UrlParserTest : public ::testing::Test
{
};

TEST_F(UrlParserTest, ParsesASimpleUrl)
{
    std::string url = "https://test.com/";

    const auto urlObject = apigen::parseUrl(url);

    ASSERT_EQ(urlObject.host, "test.com");
    ASSERT_EQ(urlObject.path, "/");
    ASSERT_EQ(urlObject.scheme, "https");

}

TEST_F(UrlParserTest, ParsesASimplePathUrl)
{
    std::string url = "https://test.com/something";

    const auto urlObject = apigen::parseUrl(url);

    ASSERT_EQ(urlObject.host, "test.com");
    ASSERT_EQ(urlObject.path, "/something");
    ASSERT_EQ(urlObject.scheme, "https");

}

TEST_F(UrlParserTest, ParsesASimplePathUrlWithQuery)
{
    std::string url = "https://test.com/something?item=2&item4=344";

    const auto urlObject = apigen::parseUrl(url);

    ASSERT_EQ(urlObject.host, "test.com");
    ASSERT_EQ(urlObject.path, "/something");
    ASSERT_EQ(urlObject.scheme, "https");

}

TEST_F(UrlParserTest, FailedToFindScheme)
{
    std::string url = "test.com/";

    ASSERT_THROW(apigen::parseUrl(url), UrlParseError);

    url = "//test.com/";

    ASSERT_THROW(apigen::parseUrl(url), UrlParseError);

    url = "://test.com/";

    ASSERT_THROW(apigen::parseUrl(url), UrlParseError);
}
