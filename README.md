# HARpoon
- a system to convert network traffic into a API wrapper.

## Description
- this is a project for me mainly to learn more on C++ since its been a long time.
- The idea behind this is to take different formats of API traffic and convert that data
into transactions to be put into a wrapper for different languages


# HARpoon --- Application Structure and Pipeline

> \*\*Purpose:\*\* This document records the current design of `HARpoon`, a
> C++ project that reads captured HTTP traffic (starting with HAR
> files), analyzes it into an API definition, and is intended to
> generate client wrappers such as TypeScript.
>
> \*\*Status note:\*\* This is a working architecture document based on the
> code and project structure discussed so far. Analyzer details reflect
> the current `APIAnalyzer` implementation shared during development.
> Items marked as planned or conceptual are not claimed to be
> implemented.

## 1\. High-level goal

`apigen` converts observed HTTP requests and responses into a reusable
API description. That description can then be used to generate client
code.

``` text
HAR file
   |
   v
HarParser
   |
   v
HttpDocument
   |
   v
APIAnalyzer  <--- parseUrl() parses each request URL
   |
   v
ApiDefinition
   |
   v
TypeScriptGenerator
   |
   v
Generated TypeScript API client
```

The architecture separates: - **Parsing:** convert an input format into
common HTTP data structures. - **URL parsing:** split a URL into
components and query parameters. - **Analysis:** infer endpoint patterns
and collect observations across requests. - **API model:** hold the
inferred API in a generator-independent format. - **Generation:**
translate the API model into a target language.

This separation makes it possible to add input formats and output
languages without rewriting the entire pipeline.

## 2\. Main source areas

The paths below reflect the organization used in the current code and
project discussions. This is a conceptual tree, not a verified
exhaustive listing of every repository file.

``` text
apiparser/
├── include/
│   └── apigen/
│       ├── analyzer/
│       │   ├── api\_analyzer.hpp
│       │   └── url\_parser.hpp
│       ├── api/
│       │   ├── definition.hpp
│       │   └── endpoint.hpp
│       └── core/
│           └── types.hpp
├── src/
│   ├── analyzer/
│   │   ├── api\_analyzer.cpp
│   │   └── url\_parser.cpp
│   ├── parser/
│   │   └── har\_parser.cpp
│   └── generator/
│       └── typescript\_generator.cpp
├── tests/
│   ├── analyzer\_test.cpp
│   └── ...
├── CMakeLists.txt
└── build/
```

The analyzer and API headers shown above are part of the current design.
Confirm exact locations and filenames for every parser/generator file
against the repository.

## 3\. Core data model

These types form the common interface between parsing, analysis, and
generation.

### `HttpDocument`

Represents a complete parsed input document. - `transactions`:
collection of `HttpTransaction` records. - The analyzer iterates over
these transactions to discover endpoints and accumulate examples.

### `HttpTransaction`

Represents one observed HTTP exchange. - `request`: the captured
request. - `response`: optional response, since some captures may not
contain one.

A transaction is the unit of evidence passed to the analyzer.

### `HttpRequest`

Represents the request side of a transaction. Fields discussed so far
include: - `method`: HTTP method, such as `GET` or `POST`. - `url`:
original request URL. - `headers`: captured request headers. - `query`:
query information, depending on the current model version. -
`contentType`: request body content type, when known. - `body`: request
payload, according to the current type definition.

The analyzer parses the URL and copies the request content type and body
into an `ApiRequest`.

### `HttpResponse`

Represents the response side of a transaction. Fields discussed so far
include: - `statusCode`: HTTP status, such as `200`, `401`, or `404`. -
`headers`: captured response headers. - `contentType`: response content
type, when known. - `body`: optional response payload.

The analyzer groups responses by status code within an endpoint and
stores distinct response bodies as examples.

### `ParsedUrl`

Produced by `parseUrl(...)`. The current analyzer uses: - `scheme`: URL
scheme, such as `https`. - `host`: hostname and any represented host
component. - `path`: URL path without its query string. -
`queryParameters`: map from each query parameter name to a vector of
values. A vector supports repeated query keys.

### `ApiDefinition`

The generator-independent result of analysis. - `baseUrl`: scheme and
host taken from the first observed URL. - `endpoints`: discovered API
endpoints.

Downstream generators should consume this model rather than parse HAR
files themselves.

### `ApiEndpoint`

Represents one inferred endpoint, identified by HTTP method and
normalized path. - `method`: HTTP method. - `path`: normalized path,
e.g. `/users/{userId}`. - `pathParameters`: inferred path parameters and
their examples. - `queryParameters`: query parameters and their
examples. - `requests`: observed request payloads/content types for the
endpoint. - `responses`: observed responses grouped by status code.

### `ApiParameter`

Represents a URL parameter. - `name`: parameter name, such as
`userId`. - `location`: where the parameter occurs; the current analyzer
uses `Path` and `Query`. - `examples`: distinct observed values.

Path parameters (`/users/123`) and query parameters (`?page=2`) are
stored separately.

### `ApiRequest`

Represents an observed request payload associated with an endpoint. The
analyzer currently copies the transaction request's content type and
body into this model. Multiple observations are retained so future
analysis can infer request-body schemas.

### `ApiResponse`

Represents the observed response for a status code. - `statusCode`:
response status. - `contentType`: observed content type. - `examples`:
distinct response bodies observed for that status.

Keeping responses separate by status code helps prevent success and
error payloads from being merged into one schema.

## 4\. Component responsibilities and methods

### `HarParser`

**Responsibility:** parse a HAR input file and convert its entries into
the common HTTP document model.

Conceptually, the parser has an operation like:

``` cpp
HttpDocument HarParser::parse(const std::filesystem::path\& path);
```

Confirm the exact signature against the current header.

Expected responsibilities: 1. Read the HAR JSON. 2. Extract each entry's
request and response. 3. Populate method, URL, headers, content type,
and body where available. 4. Populate response status, headers, content
type, and body where available. 5. Return an `HttpDocument` containing
the parsed transactions.

The parser should preserve what was captured. It should not infer
templates such as `/users/{userId}`; that is the analyzer's job.

### `parseUrl(...)`

**Responsibility:** parse one URL into a `ParsedUrl`.

The analyzer calls it for each transaction URL. It provides the
components used to determine the base URL, raw path, and query
parameters/values. It should handle URL syntax, not decide whether a
path segment represents an ID.

### `APIAnalyzer::analyze(const HttpDocument\& document)`

**Responsibility:** convert raw transaction observations into an
`ApiDefinition`.

The current implementation uses two passes.

#### Pass 1 --- Collect observations

For each transaction: 1. Parse the request URL with `parseUrl`. 2. Split
its path into segments. 3. Store a reference to the transaction, its
parsed URL, and its path segments. 4. Set `definition.baseUrl` from the
first observed URL if it is still empty.

The analyzer needs to compare paths across the input before it can infer
whether a string segment is dynamic.

#### Pass 2 --- Infer paths and aggregate endpoint data

For each collected observation: 1. Examine its path segments. 2. Treat
numeric and UUID-shaped segments as path parameters. 3. Consult a
static-route allowlist to avoid automatically treating common route
words---such as `me`, `settings`, `search`, `login`, and `register`---as
parameters. 4. Compare candidate string segments across observations to
infer dynamic segments. 5. Name parameters based on the preceding
resource segment. For example, a segment after `users` becomes `userId`;
one after `posts` becomes `postId`. 6. Normalize the path,
e.g. `/users/123` to `/users/{userId}`. 7. Find or create the endpoint
matching the HTTP method and normalized path. 8. Add query parameters
and their observed examples. 9. Add path parameter examples, avoiding
duplicates. 10. Store an `ApiRequest` containing the observed request
content type and body. 11. If a response exists, find or create the
response entry for its status code and add a distinct body example.

The analyzer currently stores request/response examples. It does not yet
infer full JSON object schemas from those bodies.

#### Private analyzer helpers

\---

Method/helper                       Responsibility

\---

`findOrCreateEndpoint(...)`         Return the endpoint with the same
method and normalized path, or
create it.

`findOrCreateResponse(...)`         Return the response with the same
status code, or create it. This
lets examples accumulate by status.

`findQueryParameter(...)`           Find a query parameter by name on
an endpoint, or return `nullptr`.

`addQueryParameters(...)`           Merge query parameters from a
`ParsedUrl`, adding distinct
observed values.

`splitPath(...)`                    Split a path into individual
non-empty segments.

`joinPath(...)`                     Join normalized path segments into
a slash-prefixed path.

`isNumericSegment(...)`             Check whether a non-empty segment
contains only digits.

`isUuidSegment(...)`                Check whether a segment matches the
expected 36-character UUID layout.

`isStaticRouteSegment(...)`         Check whether a segment is in the
static-route allowlist.

`makeComparisonKey(...)`            Build a comparison key for a
candidate segment; the current
version masks that candidate and
numeric/UUID segments elsewhere.

`parameterNameForResource(...)`     Convert a resource segment into a
parameter name using separator
handling, camel-casing, and basic
plural reduction.
\---

Some helpers are in an anonymous namespace in `api\_analyzer.cpp`, so
they are implementation details rather than public API.

### `TypeScriptGenerator`

**Responsibility:** generate TypeScript client code from
`ApiDefinition`.

The intended boundary is conceptually similar to:

``` cpp
// Conceptual only; confirm the exact current signature.
TypeScriptGenerator generator;
auto output = generator.generate(definition);
```

The generator should consume the analyzed API model rather than parse
HAR data. Once schema inference exists, it can use inferred request and
response types to emit TypeScript interfaces/types and typed endpoint
methods. The exact current generator behavior and signature need to be
verified against its header and implementation.

### `APIAnalyzer` versus `url\_parser`

Keep their responsibilities separate: - `url\_parser` answers: **What are
the components of this URL?** - `APIAnalyzer` answers: **What API
endpoint and parameters do these observations suggest?**

For example, the URL parser returns `/users/alex` as a path. The
analyzer can decide, based on multiple observations, whether `alex` is
likely a `userId`.

## 5\. End-to-end example

Suppose the captured traffic contains:

``` text
GET https://api.example.com/users/123?page=1
GET https://api.example.com/users/456?page=2
```

### A. Parse the input

`HarParser` produces an `HttpDocument` with two transactions.

### B. Parse each URL

`parseUrl(...)` extracts data equivalent to:

``` text
scheme: https
host: api.example.com
path: /users/123
queryParameters:
  page -> \["1"]
```

The second URL produces `/users/456` and `page -> \["2"]`.

### C. Infer the path

`APIAnalyzer` recognizes the numeric segments as dynamic and normalizes
both paths to:

``` text
/users/{userId}
```

### D. Merge observations

Because the method and normalized path match,
`findOrCreateEndpoint(...)` returns the same endpoint for both requests.

The endpoint accumulates:

``` text
method: GET
path: /users/{userId}

pathParameters:
  userId
    examples: \["123", "456"]

queryParameters:
  page
    examples: \["1", "2"]
```

Both request observations are retained. If responses exist, response
bodies are recorded in response entries grouped by status code.

### E. Return the API model

The analyzer returns an `ApiDefinition` containing the base URL and
merged endpoints.

### F. Generate client code

A generator such as `TypeScriptGenerator` can consume the definition and
produce client code. Full typed generation depends on further
schema-inference work.

## 6\. Current capabilities and known limitations

### Implemented in the current analyzer code

* Parses request URLs with `parseUrl`.
* Sets the base URL from the first observed URL.
* Splits paths into segments.
* Recognizes numeric and UUID-shaped path segments.
* Uses a static-route allowlist.
* Attempts evidence-based inference for changing string path segments.
* Names parameters from preceding resource segments.
* Aggregates endpoints by HTTP method and normalized path.
* Collects path parameter examples without duplicates.
* Collects query parameter examples without duplicates.
* Retains observed request content types and bodies.
* Groups responses by status code and retains distinct response-body
examples.

### Known limitations to address

* **Multiple changing string segments:** the current comparison-key
approach can struggle with paths such as `/orgs/acme/projects/alpha`
and `/orgs/contoso/projects/beta`.
* **Static-route detection:** a fixed list cannot cover every API's
route vocabulary.
* **Parameter naming:** basic plural reduction is heuristic and will
not handle every resource name.
* **Multiple hosts:** the current implementation uses the first
observed URL as `baseUrl`; inputs with multiple hosts may need
validation or grouping.
* **Content-type variants:** when observations for the same response
status have different content types, the current assignment may
overwrite the earlier value instead of representing variants.
* **Schema inference:** JSON bodies are stored as examples, but full
schemas are not inferred yet.
* **Generator behavior:** exact output depends on the current
generator implementation and needs its own documentation.

## 7\. Build and test workflow

The project uses CMake and GoogleTest. Run from the repository root:

``` bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Configure the build directory if needed:

``` bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Useful analyzer regression cases include: - Numeric and UUID path
parameters. - String parameters inferred from multiple observations. -
Static routes such as `/users/me` remaining static. - Nested path
parameters. - Multiple changing string segments in one path. - Query
parameters with repeated/changing values. - Multiple response status
codes for one endpoint. - Duplicate response bodies not being stored
repeatedly. - One string-path observation not being sufficient to infer
a dynamic segment.

## 8\. Recommended next milestones

1. Finish path inference for multiple changing segments while keeping
static routes distinct.
2. Lock the inference rules in regression tests.
3. Add JSON schema inference for primitives, objects, arrays,
nullability, and optional properties.
4. Keep response schemas separated by status code (for example, `200`
success versus `401` error).
5. Connect inferred schemas to `TypeScriptGenerator`.
6. Add more input formats by converting them into the same common HTTP
model.
7. Document generated-client conventions: authentication, base URL
configuration, query serialization, request bodies, and error
handling.

## 9\. Design principle

Preserve the modular pipeline:

* Input parsers create common HTTP records.
* The URL parser parses syntax.
* The analyzer infers API meaning from multiple observations.
* `ApiDefinition` is the stable intermediate representation.
* Generators translate that representation into language-specific
output.

This lets the project support more capture formats and generated
languages without coupling every component to every other component.


