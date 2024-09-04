# swifties - Web Server

## Navigating The Codebase

The bulk of the codebase is divided into several folders for modularity. For simplictly, this guide will detail only those which are needed to interact with the core functionality of the server and it's deployment: *src, docker, tests, logs,* and CMakeLists.txt.

### src

The src directory maintains the majority of the web server's implementation. The program's entry can be found in server_main.cc, which handles config validation and server instantiation.

Several other files exist at the first level of /src. A high-level overview for each follows:

- `config_parser`: Contains several methods used to parse and validate the contents of an Nginx Configuration file. The `Parse()` method can be viewed as the entry point. Note that `get_config_port()` is leveraged to extract the port number.

- `server`: Implements the server class, which orchestrates the reception of incoming connections and the management of sessions within the web server. It utilizes Boost.Asio for asynchronous I/O operations and TCP/IP socket handling. The server class initializes an acceptor to listen on a specified port and delegates incoming connections to session handlers for processing.

- `session`: Implements the session class, which represents a single connection within the web server. Sessions handle the reading and writing of data over TCP sockets using Boost.Asio. They are responsible for parsing incoming HTTP requests, dispatching them to the appropriate request handlers, and sending back HTTP responses. The session class utilizes asynchronous I/O operations to handle multiple connections concurrently, ensuring efficient resource utilization. It also integrates a logging mechanism to record various events during the session lifecycle, aiding in debugging and monitoring.

- `request_handler_dispatcher`: Implements the RequestHandlerDispatcher class, which manages the mapping between request URIs and corresponding request handler objects. It plays a crucial role in routing incoming HTTP requests to the appropriate handlers based on the requested URI path. The getRequestHandler method retrieves the appropriate request handler object for a given URI path. The initRequestHandlers method initializes request handlers by parsing the server configuration block and registering handler paths based on the specified directives. The registerPath method registers a handler path with its corresponding handler type and configuration. 

- `logger`: Creates a logger object which is instantiated in various other parts of the implementation for debugging.

The src folder also contains an http folder responsible for parsing the validity of HTTP requests sent to the server. A brief description of these files follows:

- `request_parser`: Implements the RequestParser class, which is responsible for parsing raw HTTP request messages into structured representations. It utilizes Boost.Beast library to efficiently handle HTTP message parsing. The RequestParser class encapsulates the logic for processing raw HTTP request data, validating its format, and constructing a request object that represents the parsed information.

- `mime_types`: provides functionality for mapping file extensions to MIME (Multipurpose Internet Mail Extensions) types. MIME types are essential for web servers to correctly identify the content type of files being served, enabling proper rendering and interpretation by web browsers. The extension_to_type function within the mime_types namespace is responsible for converting a file extension into the corresponding MIME type.

The src folder also contains a request_handler folder responsible for implementing the various different handlers utilized to generate and return a response. Currently, the server implements disntinct handlers for static files (`request_handler_static`), echoed requests (`request_handler_echo`), and 404 unmatched prefixes.

### tests

The tests directory maintains a set of unit tests and integration test scripts used to ensure expected functionality of various components of the server. Each file tests a corresponding module from src/ with a matching prefix. For instance, `session_test.cc` writes several unit tests for `src/session.cc`. Since many test cases for the config_parser require opening local files, these files can be found under `/config_parser_tests` for easier access.

Note that integration_test.sh and test.sh run the server and transmit dummy requests to ensure proper response.

### docker

The docker directory consists primarily of project boilerplate. The combination of files present are used to spin up a production container with the proper dependencies installed (`base.Dockerfile`), a sequence of commands to build and test in production (`cloudbuild.yaml`), directory navigation for coverage reports (`coverage.Dockerfile`), and additional specification and instruction (`Dockerfile`).

### logs

The logs directory maintains a list of log files which trace the steps of the server's implementation during use. Log files are generated on a per-day-basis so long as a request is made to the server on said day. Exact timestamps, IP addresses, and message_types can be examined for each instance of the server.

### CMakeLists.txt

Used for configuring the build process of the "swifties" project. It sets up various build options, dependencies, and targets for compiling the project's source code and running tests.

## Build, Test, and Run!

### Running Unit Tests

Building and testing the server is a quick and easy process. From the root of the project, enter the following series of commands:

```
mkdir build
cd build
cmake ..
make
make test
```

At this point you should see a series of tests run in your CLI. These correspond to the unit tests from the `tests/` directory.

Note: If these commands fail, first make sure you're in the docker image. Follow the instructions from the [Assignment 1](https://www.cs130.org/assignments/1/) spec.

### Interacting With The Server

To interact with the server directly from the CLI, first build the files as specified in the above section. Then, from the /build directory, run `bin/server 8080`. Open another terminal and run `nc localhost 8080`. From here, enter a valid HTTP request. The response should return underneath!

Note: If these commands fail, first make sure you're in the docker image. Follow the instructions from the [Assignment 1](https://www.cs130.org/assignments/1/) spec. netcat must be installed locally for this to work outside the Docker image.

### Generating Coverage Reports

To generate coverage reports, first navigate back to the root project directory. From there, run the following series of commands:

```
mkdir build_coverage
cd build_coverage
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make coverage
```

A summary of both line and branch coverage should appear in the output. To view a more in-depth analysis of the coverage report, open `/build_coverage/report/index.html` in the browser.


## Adding Request Handlers
1. Create the appropriate files for the new request handler. A request handler should declare a public member `void handleRequest(const Request &request_, Response *response_) noexcept override;`. In the `handleRequest()` definition the response message should be built by referencing the attributes of the `boost::beast::http::response` object. For example in `RequestHandlerEcho()`:
```
void RequestHandlerEcho::handleRequest(const Request &request_,
                                       Response *response_) noexcept {
  std::cout << "RequestHandlerEcho::handleRequest()" << std::endl;

  response_->version(request_.version());
  response_->result(http::status::ok);
  // Echo back the request as the response body
  response_->body() = requestToString(request_);
  response_->set(http::field::content_type, "text/plain");
  response_->content_length(response_->body().size());
  // Prepare the payload to ensure headers and body are correctly formatted
  response_->prepare_payload();
}
```
2. Define the handler type in the configuration file with its corresponding url. For example to declare the handler type `EchoRequest`, we have the following in our configuration file:
```
# Define location block for handling echo requests
handler EchoHandler {
  url /echo;
    }
```
3. In the request dispatcher, in the function `bool RequestHandlerDispatcher::registerPath(PathUri path_uri, const std::string &handler_type, const NginxConfig &config)` add an `else if` statement so if the `handler_type` matches the declaration in the configuration file then the `handlers[path_uri]` creates a new shared pointer   to an instance of the specific request handler's class. For example, here is how it would look for an echo request:
```
else if (handler_type == "EchoHandler")
  handlers_[path_uri] = std::make_shared<RequestHandlerEcho>();
```
4. Add the path to new request handler file to the `CMakeLists.txt` in `add_library(request_handler)`