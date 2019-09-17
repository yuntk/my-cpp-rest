#include <iostream>

#include "myrest.h"

using namespace std;
using namespace web::json;

int main(int argc, char *argv[]) {



    value json_v;
//    json_v["title"] = value::string("foo");
//    json_v["i"] = value::number(8);
//    json_v["body"] = value::string("bar");
//    json_v["userId"] = value::number(1);

    value json_return = my_post("/start/tester/0/2", json_v);
    std::cout << json_return.serialize() << std::endl;
    string token = json_return.at("token").as_string();
    json_return = my_get("/oncalls",token);
    cout << json_return << endl;
    web::json::array arr =  json_return.at("calls").as_array();

//    cout << endl;
//    json_return = my_get("/get");
//    std::cout << json_return.serialize() << std::endl;

    return 0;
}