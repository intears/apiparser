#include "apigen/analyzer/api_analyzer.hpp"
#include "apigen/api/definition.hpp"
#include "apigen/api/endpoint.hpp"
#include "apigen/core/types.hpp"

namespace apigen {

    ApiDefinition APIAnalyzer::analyze(const HttpDocument& document)
    {
        ApiDefinition definiftion;

        for (const auto& transaction : document.transactions) {
            //1. parse URL
            //2. Find/create endpoint
            //3. Add request oversvation
            //4. Add response ovservation
            //5. Later: infer schemas
            //6. Later: nromalize pathing
        }


        return definiftion;
    }


    std::string APIAnalyzer::parseUrl(const HttpTransaction& transaction) {
        std::string url = transaction.request.url;
        return url; // TODO CHANGE THIS shit to be correct
    }

    ApiEndpoint APIAnalyzer::createEndpoint(const HttpTransaction& transaction) {
       ApiEndpoint endpoint;

    }

}
