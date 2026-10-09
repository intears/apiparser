#include <gtest/gtest.h>

#include "apigen/analyzer/api_analyzer.hpp"
#include "apigen/api/definition.hpp"
#include "apigen/core/types.hpp"

#include <algorithm>
#include <string>

class AnalyzerTest : public ::testing::Test {
protected:
  apigen::APIAnalyzer analyzer;
};

TEST_F(AnalyzerTest, EmptyDocumentProducesNoEndpoints) {
  const apigen::HttpDocument document;

  const auto definition = analyzer.analyze(document);

  EXPECT_TRUE(definition.endpoints.empty());
}

TEST_F(AnalyzerTest, GroupsTransactionsAndAccumulatesObservations) {
  apigen::HttpDocument document;

  // First request: GET /users?sort=name -> 200
  apigen::HttpTransaction first;
  first.request.method = "GET";
  first.request.url = "https://example.com/users?sort=name";

  apigen::HttpResponse firstResponse;
  firstResponse.statusCode = 200;
  firstResponse.body = R"({"users":[]})";
  first.response = firstResponse;

  document.transactions.push_back(first);

  // Second request: same endpoint, different query value -> 200
  apigen::HttpTransaction second;
  second.request.method = "GET";
  second.request.url = "https://example.com/users?sort=date";

  apigen::HttpResponse secondResponse;
  secondResponse.statusCode = 200;
  secondResponse.body = R"({"users":[{"id":1}]})";
  second.response = secondResponse;

  document.transactions.push_back(second);

  // Third request: same endpoint, different response status -> 404
  apigen::HttpTransaction third;
  third.request.method = "GET";
  third.request.url = "https://example.com/users?sort=name";

  apigen::HttpResponse thirdResponse;
  thirdResponse.statusCode = 404;
  thirdResponse.body = R"({"error":"not found"})";
  third.response = thirdResponse;

  document.transactions.push_back(third);

  const auto definition = analyzer.analyze(document);

  ASSERT_EQ(definition.endpoints.size(), 1);

  const auto &endpoint = definition.endpoints.front();

  EXPECT_EQ(endpoint.method, "GET");
  EXPECT_EQ(endpoint.path, "/users");
  EXPECT_EQ(endpoint.requests.size(), 3);
  EXPECT_EQ(endpoint.queryParameters.size(), 1);

  const auto &parameter = endpoint.queryParameters.front();

  EXPECT_EQ(parameter.name, "sort");
  EXPECT_EQ(parameter.location, apigen::ParameterLocation::Query);
  EXPECT_EQ(parameter.examples.size(), 2);

  EXPECT_NE(
      std::find(parameter.examples.begin(), parameter.examples.end(), "name"),
      parameter.examples.end());

  EXPECT_NE(
      std::find(parameter.examples.begin(), parameter.examples.end(), "date"),
      parameter.examples.end());

  ASSERT_EQ(endpoint.responses.size(), 2);

  const auto response200 =
      std::find_if(endpoint.responses.begin(), endpoint.responses.end(),
                   [](const apigen::ApiResponse &response) {
                     return response.statusCode == 200;
                   });

  ASSERT_NE(response200, endpoint.responses.end());
  EXPECT_EQ(response200->examples.size(), 2);

  const auto response404 =
      std::find_if(endpoint.responses.begin(), endpoint.responses.end(),
                   [](const apigen::ApiResponse &response) {
                     return response.statusCode == 404;
                   });

  ASSERT_NE(response404, endpoint.responses.end());
  EXPECT_EQ(response404->examples.size(), 1);
}
