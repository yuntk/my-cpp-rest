//
// Created by tk on 19. 9. 17..
//

#ifndef CODETEST_MYREST_H
#define CODETEST_MYREST_H

#define CLIENT_URI "http://localhost:8000/"
#define DEBUG true

#include <cpprest/http_client.h>
#include <cpprest/json.h>
//#include <cpprest/filestream.h>

//using namespace utility;                    // Common utilities like string conversions
//using namespace web;                        // Common features like URIs.
//using namespace web::http;                  // Common HTTP functionality
//using namespace web::http::client;          // HTTP client features
//using namespace concurrency::streams;       // Asynchronous streams

//std::string CLIENT_URI = "https://httpbin.org/";

web::json::value my_post(const std::string& uri, const web::json::value& json_, const std::string& header = "") {
    web::json::value json_return;
    web::http::client::http_client client(CLIENT_URI);
    web::http::http_request req(web::http::methods::POST);
    req.set_request_uri(uri);
    req.set_body(json_);
    if(DEBUG) std::cout << "(POST)" << "uri : " << uri << " header : " << header << "json : " << json_ << "\n";
    if (!header.empty()) req.headers().add("X-Auth-Token", header);
    client.request(req)
            .then([](const web::http::http_response &response) {
                if(DEBUG) std::cout << "STATUS : " << response.status_code() << "\n";
                return response.extract_json();
            })
            .then([&json_return](const pplx::task<web::json::value> &task) {
                try {
                    json_return = task.get();
                }
                catch (const web::http::http_exception &e) {
                    std::cout << "error " << e.what() << std::endl;
                }
            })
            .wait();
    return json_return;
}

web::json::value my_get(const std::string& uri, const std::string& header = "") {
    web::json::value json_return;
    web::http::client::http_client client(CLIENT_URI);
    web::http::http_request req(web::http::methods::GET);
    req.set_request_uri(uri);
    if(DEBUG) std::cout << "(GET)" << "uri : " << uri << " header : " << header << "\n";
    if (!header.empty()) req.headers().add("X-Auth-Token", header);
    client.request(req)
            .then([](const web::http::http_response &response) {
                if(DEBUG) std::cout << "STATUS : " << response.status_code() << "\n";
                return response.extract_json();
            })
            .then([&json_return](const pplx::task<web::json::value> &task) {
                try {
                    json_return = task.get();
                }
                catch (const web::http::http_exception &e) {
                    std::cout << "error " << e.what() << std::endl;
                }
            })
            .wait();
    return json_return;
}


#endif //CODETEST_MYREST_H
