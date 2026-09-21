#include <gtest/gtest.h>

#include <filesystem>

#include "apigen/parser/har_parser.hpp"

namespace fs = std::filesystem;

class HarParserTest : public ::testing::Test
{
protected:
    apigen::HarParser parser;

    fs::path fixture(const std::string& name)
    {
        return fs::path(TEST_DATA_DIR) / name;
    }
};

TEST_F(HarParserTest, ParsesRequestMethodAndUrl)
{
    const auto document =
        parser.parse(fixture("basic.har"));

    ASSERT_EQ(document.transactions.size(), 1);

    const auto& request =
        document.transactions[0].request;

    EXPECT_EQ(request.method, "GET");

    EXPECT_EQ(
        request.url,
        "https://api.example.com/users?id=123"
    );
}

TEST_F(HarParserTest, ParsesRequestHeaders)
{
    const auto document =
        parser.parse(fixture("basic.har"));

    ASSERT_EQ(document.transactions.size(), 1);

    const auto& headers =
        document.transactions[0].request.headers;

    ASSERT_EQ(headers.size(), 2);

    EXPECT_EQ(
        headers.at("Authorization"),
        "Bearer test-token"
    );

    EXPECT_EQ(
        headers.at("Accept"),
        "application/json"
    );
}

TEST_F(HarParserTest, ParsesMultipleTransacations)
{
    const auto document = parser.parse(fixture("multiple.har"));

    ASSERT_EQ(document.transactions.size(), 3);

    EXPECT_EQ(
        document.transactions[0].request.method,
        "GET"
    );

    EXPECT_EQ(
        document.transactions[1].request.method,
        "POST"
    );

    EXPECT_EQ(
        document.transactions[2].request.method,
        "DELETE"
    );
}

TEST_F(HarParserTest, ThrowsWhenFileDoesNotExist)
{
    EXPECT_THROW(
        parser.parse(fixture("does-not-exist.har")),
        std::runtime_error
    );
}


TEST_F(HarParserTest, ReportsMissingFile)
{
    try
    {
        parser.parse(fixture("does-not-exist.har"));

        FAIL() << "Expected parser to throw";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_NE(
            std::string(e.what()).find(
                "Failed to open HAR file"
            ),
            std::string::npos
        );
    }
}

TEST_F(HarParserTest, ParseMutipleResponseTransacations)
{
    const auto document = parser.parse(fixture("response.har"));

    ASSERT_EQ(document.transactions.size(), 3);

    EXPECT_EQ(
        document.transactions[0].request.method,
        "GET"
    );

    EXPECT_EQ(
        document.transactions[1].request.method,
        "POST"
    );

    EXPECT_EQ(
        document.transactions[2].request.method,
        "GET"
    );

    ASSERT_EQ(
        document.transactions[0].response->headers.size(), 2
    );

    ASSERT_EQ(
        document.transactions[1].response->headers.size(), 2
    );

    ASSERT_EQ(
        document.transactions[2].response->headers.size(), 1
    );

    EXPECT_EQ(document.transactions[0].response->statusCode, 200);
    EXPECT_EQ(document.transactions[1].response->statusCode, 201);
    EXPECT_EQ(document.transactions[2].response->statusCode, 404);

    EXPECT_EQ(document.transactions[0].response->statusText, "OK");
    EXPECT_EQ(document.transactions[1].response->statusText, "Created");
    EXPECT_EQ(document.transactions[2].response->statusText, "Not Found");


}

TEST_F(HarParserTest, ParsesRequestBody)
{
    const auto document = parser.parse(fixture("response.har"));

    ASSERT_EQ(document.transactions.size(), 3);

    const auto& request =
        document.transactions[1].request;

    ASSERT_TRUE(request.body.has_value());

    EXPECT_EQ(
        request.body.value(),
        R"({"name":"Alice","email":"alice@example.com"})"
    );
}

TEST_F(HarParserTest, ParsesResponseBody)
{
    const auto document = parser.parse(fixture("response.har"));

    ASSERT_EQ(document.transactions.size(), 3);

    const auto& response =
        document.transactions[1].response;

    ASSERT_TRUE(response.has_value());

    ASSERT_TRUE(response->body.has_value());

    EXPECT_EQ(
        response->body.value(),
        R"({"id":124,"name":"Alice","email":"alice@example.com"})"
    );
}

TEST_F(HarParserTest, GetContentTypeOfRequest)
{
    const auto document = parser.parse(fixture("response.har"));

    const auto& request =
        document.transactions[1].request;


    EXPECT_EQ(
        request.contentType,
        "application/json"
    );
}
