#pragma once

#include "apigen/analyzer/api_analyzer.hpp"
#include "apigen/api/definition.hpp"
#include "apigen/core/types.hpp"

class IAnalyzer {
public:
    virtual ~IAnalyzer() = default;

    virtual apigen::APIAnalyzer analyze(
            const apigen::HttpDocument& document
    ) = 0 ;

};




