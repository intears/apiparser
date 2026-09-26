#include <gtest/gtest.h>

#include "apigen/analyzer/url_parser.hpp"
#include "apigen/core/error.hpp"

class UrlParserTest : public ::testing::Test {};

TEST_F(UrlParserTest, ParsesASimpleUrl) {
  std::string url = "https://test.com/";

  const auto urlObject = apigen::parseUrl(url);

  ASSERT_EQ(urlObject.host, "test.com");
  ASSERT_EQ(urlObject.path, "/");
  ASSERT_EQ(urlObject.scheme, "https");
}

TEST_F(UrlParserTest, ParsesASimplePathUrl) {
  std::string url = "https://test.com/something";

  const auto urlObject = apigen::parseUrl(url);

  ASSERT_EQ(urlObject.host, "test.com");
  ASSERT_EQ(urlObject.path, "/something");
  ASSERT_EQ(urlObject.scheme, "https");
}

TEST_F(UrlParserTest, ParsesASimplePathUrlWithQuery) {
  std::string url = "https://test.com/something?item=2&item4=344";

  const auto urlObject = apigen::parseUrl(url);

  ASSERT_EQ(urlObject.host, "test.com");
  ASSERT_EQ(urlObject.path, "/something");
  ASSERT_EQ(urlObject.scheme, "https");
}

TEST_F(UrlParserTest, FailedToFindScheme) {
  std::string url = "test.com/";

  ASSERT_THROW(apigen::parseUrl(url), UrlParseError);

  url = "//test.com/";

  ASSERT_THROW(apigen::parseUrl(url), UrlParseError);

  url = "://test.com/";

  ASSERT_THROW(apigen::parseUrl(url), UrlParseError);
}

TEST_F(UrlParserTest, ParsesQueryParameters) {
  const auto url =
      apigen::parseUrl("https://api.example.com/users?id=123&sort=name");

  ASSERT_EQ(url.queryParameters.size(), 2);

  ASSERT_EQ(url.queryParameters.at("id").size(), 1);

  EXPECT_EQ(url.queryParameters.at("id")[0], "123");

  EXPECT_EQ(url.queryParameters.at("sort")[0], "name");
}

TEST_F(UrlParserTest, ParsesRepeatedQueryParameters)
{
    const auto url =
        apigen::parseUrl(
            "https://api.example.com/users?tag=cpp&tag=api"
        );

    ASSERT_EQ(
        url.queryParameters.at("tag").size(),
        2
    );

    EXPECT_EQ(
        url.queryParameters.at("tag")[0],
        "cpp"
    );

    EXPECT_EQ(
        url.queryParameters.at("tag")[1],
        "api"
    );
}
