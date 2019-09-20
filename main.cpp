#include <iostream>
#include <vector>
#include <set>

#include "myrest.h"


using namespace std;
using namespace web::json;

int find_closest_call(set<tuple<int, int, int>> &call_set, int cur_floor) {
    if(call_set.empty()){
        return 3;
    }
    auto ret = call_set.lower_bound(make_tuple(cur_floor, 0, 0));
    if (ret == call_set.end()) {
        return get<0>(*(--ret));
    }
    if (ret == call_set.begin()) return get<0>(*ret);
    int a = get<0>(*(--ret));
    int b = get<0>(*ret);
    return cur_floor - a > b - cur_floor ? b : a;
}

bool is_there_upward_caller(set<tuple<int, int, int>> &call_set, int cur_floor) {
    //이미 콜이 있음을 확인한 경우만 call
    if(call_set.empty()) return false;
    auto iter = call_set.lower_bound(make_tuple(cur_floor, 0, 0));
    int start, end;
    while (iter != call_set.end() && get<0>(*iter) == cur_floor) {
        tie(start, end, ignore) = *iter;
        iter++;
        if (end > cur_floor) true;
    }
    return false;
}

bool is_there_downward_caller(set<tuple<int, int, int>> &call_set, int cur_floor) {
    //이미 콜이 있음을 확인한 경우만 call
    if(call_set.empty()) return false;
    auto iter = call_set.lower_bound(make_tuple(cur_floor, 0, 0));
    int start, end;
    while (iter != call_set.end() && get<0>(*iter) == cur_floor) {
        tie(start, end, ignore) = *iter;
        iter++;
        if (end < cur_floor) true;
    }
    return false;
}


vector<value> enter_list(set<tuple<int, int, int>> &call_set, int cur_floor, string direction, int people) {
    vector<value> ret;
    if(call_set.empty()) return ret;
    auto iter = call_set.lower_bound(make_tuple(cur_floor, 0, 0));
    int start, end, id;
    while (iter != call_set.end() && ret.size() != people) {
        tie(start, end, id) = *iter;
        if (start == cur_floor) {
            if (direction == "UP") {
                if (end > cur_floor) {
                    bool duplicate = false;
                    for(auto a: ret){
                        if(a.as_integer() == id) duplicate = true;
                    }
                    if(!duplicate)ret.emplace_back(id);
                    ret.emplace_back(id);
                    iter = call_set.erase(iter);
                } else iter++;

            } else {
                if (end <= cur_floor) {
                    bool duplicate = false;
                    for(auto a: ret){
                        if(a.as_integer() == id) duplicate = true;
                    }
                    if(!duplicate)ret.emplace_back(id);
                    iter = call_set.erase(iter);
                } else iter++;
            }
        } else return ret;
    }
    return ret;
}

//손님중 가장 가까운 목적지가 어디인가?
int nearest_upward_dest(value passengers, int cur_floor) {
    vector<int> end_list;
    for (auto e : passengers.as_array()) {
        end_list.push_back(e.at("end").as_integer());
    }
    sort(end_list.begin(), end_list.end());
    for (int n : end_list) {
        if (n >= cur_floor) return n;
    }
    return 0;
}

int nearest_down_dest(value passengers, int cur_floor) {
    vector<int> end_list;
    for (auto e : passengers.as_array()) {
        end_list.push_back(e.at("end").as_integer());
    }
    sort(end_list.rbegin(), end_list.rend());
    for (int n : end_list) {
        if (n <= cur_floor) return n;
    }
    return 0;
}

vector<value> exit_passenger_list(value passengers, int floor) {
    vector<value> ret;
    for (value a : passengers.as_array()) {
        if (a.at("end").as_integer() == floor) ret.push_back(a.at("id").as_integer());
    }
    return ret;
}

int main(int argc, char *argv[]) {

    /*
     * 토큰 얻어오기 문제랑 엘베 번호 세팅
     */
    value json_return = my_post("/start/tester/2/4", (value) "");
    std::cout << json_return.serialize() << std::endl;
    string token = json_return.at("token").as_string();


    web::json::array *calls;
    web::json::array *elevators;
    bool is_end = false;
    vector<value> elv_cmd;
    value elv;
    //start,end,id
    set<tuple<int, int, int>> call_set;
    //pass_size, floor, status, id, passengers
    int pass_size, floor, id;
    string status;
    value passengers;
    vector<tuple<int, int, string, int, value>> elevator_list;

    while (!is_end) {
        /*
         * oncalls, it gets the status of calls and elevators.
         * Then, I have to make commands from the status
         */

        json_return = my_get("/oncalls", token);
        cout << json_return << endl;
        calls = &json_return.at("calls").as_array();
        elevators = &json_return.at("elevators").as_array();
        is_end = json_return.at("is_end").as_bool();
        call_set.clear();

        for (auto call : *calls) {
            call_set.insert(make_tuple(
                    call.at("start").as_integer(),
                    call.at("end").as_integer(),
                    call.at("id").as_integer()));
        }
        elevator_list.clear();
        for (auto e : *elevators) {
            elevator_list.push_back(make_tuple(
                    e.at("passengers").size(),
                    e.at("floor").as_integer(),
                    e.at("status").as_string(),
                    e.at("id").as_integer(),
                    e.at("passengers")));
        }
        elv_cmd.clear();
        for (auto e : elevator_list) {
            tie(pass_size, floor, status, id, passengers) = e;
            elv = value::object();
            if (pass_size == 0) { //no passenger
                int closest_call = find_closest_call(call_set, floor);
                elv["elevator_id"] = id;
                if (status == "STOPPED") {
                    // Do something : STOP, UP, DOWN, OPEN
                    if (closest_call == floor) { //any passenger in this floor?
                        elv["command"] = value::string("OPEN");
                    } else {
                        if (closest_call > floor) elv["command"] = value::string("UP");
                        else elv["command"] = value::string("DOWN");
                    }
                } else if (status == "UPWARD") {
                    // Do something : STOP, UP
                    if (closest_call <= floor) { //any passenger in this floor?
                        elv["command"] = value::string("STOP");
                    } else {
                        elv["command"] = value::string("UP");
                    }
                } else if (status == "DOWNWARD") {
                    if (closest_call >= floor) { //any passenger in this floor?
                        elv["command"] = value::string("STOP");
                    } else {
                        elv["command"] = value::string("DOWN");
                    }
                    // Do something : STOP, DOWN
                } else if (status == "OPENED") {
                    // Do something : OPEN, CLOSE, ENTER, EXIT
                    vector<value> uplist = enter_list(call_set, floor, "UP", 8);
                    vector<value> downlist = enter_list(call_set, floor, "DOWN", 8);
                    if (uplist.empty() && downlist.empty()) {
                        elv["command"] = value::string("CLOSE");
                    } else {
                        elv["command"] = value::string("ENTER");
                        if (uplist.size() > downlist.size()) elv["call_ids"] = value::array(uplist);
                        else elv["call_ids"] = value::array(downlist);
                    }
                } else cout << "status error???";


            } else if (pass_size != 8) { //some passenger
                int closest_call = find_closest_call(call_set, floor);
                elv["elevator_id"] = id;
                if (status == "STOPPED") {
                    // Do something : STOP, UP, DOWN, OPEN
                    if (closest_call == floor) { //any passenger in this floor?
                        elv["command"] = value::string("OPEN");
                    } else { //태울 손님이 없으면 움직인다.
                        //손님중 가장 가까운 목적지가 어디인가?
                        int updest = nearest_upward_dest(passengers, floor);
                        int downdest = nearest_down_dest(passengers, floor);
                        if (updest == floor) { //내릴손님있음
                            elv["command"] = value::string("OPEN");
                        } else if (updest) {
                            if (downdest) {
                                //둘중 가까운곳으로
                                if (updest - floor > floor - downdest) elv["command"] = value::string("DOWN");
                                else elv["command"] = value::string("UP");
                            } else {
                                //updest밖에 없음
                                elv["command"] = value::string("UP");
                            }
                        } else {
                            if (downdest) {
                                elv["command"] = value::string("DOWN");
                            } else {
                                cout << "there is no calls in elv or error\n";
                            }
                        }
                    }
                } else if (status == "UPWARD") {
                    // Do something : STOP, UP
                    int updest = nearest_upward_dest(passengers, floor);
                    if (updest == floor) { //내릴손님있음
                        elv["command"] = value::string("STOP");
                    } else if (closest_call == floor) { //any passenger in this floor?
                        //올라가는 손님이냐?
                        if (is_there_upward_caller(call_set, floor)) elv["command"] = value::string("STOP");
                        else elv["command"] = value::string("UP");
                    } else if (updest) {
                        //위로가는 손님이 있어야 정상
                        elv["command"] = value::string("UP");
                    } else {
                        cout << "error downward pass in upward elv\n";
                        elv["command"] = value::string("STOP");
                    }

                } else if (status == "DOWNWARD") {

                    // Do something : STOP, DOWN
                    int downdest = nearest_down_dest(passengers, floor);
                     if (downdest == floor) { //내릴손님있음
                        elv["command"] = value::string("STOP");
                    } else if (closest_call == floor) { //any passenger in this floor?
                         //내려가는 손님이냐?
                         if (is_there_downward_caller(call_set, floor)) elv["command"] = value::string("STOP");
                         else elv["command"] = value::string("DOWN");
                     } else if (downdest) {
                        //위로가는 손님이 있어야 정상
                        elv["command"] = value::string("DOWN");
                    } else {
                        cout << "error upward pass in downward elv\n";
                        elv["command"] = value::string("STOP");
                    }

                } else if (status == "OPENED") {
                    // Do something : OPEN, CLOSE, ENTER, EXIT
                    //내릴손님부터 검색
                    vector<value> exit_list = exit_passenger_list(passengers, floor);
                    if (exit_list.size()) {
                        elv["command"] = value::string("EXIT");
                        elv["call_ids"] = value::array(exit_list);
                    } else { //내릴 손님 없음
                        vector<value> uplist = enter_list(call_set, floor, "UP", 8 - pass_size);
                        vector<value> downlist = enter_list(call_set, floor, "DOWN", 8 - pass_size);
                        if (uplist.empty() && downlist.empty()) {
                            elv["command"] = value::string("CLOSE");
                        } else {
                            elv["command"] = value::string("ENTER");
                            if (uplist.size() > downlist.size()) elv["call_ids"] = value::array(uplist);
                            else elv["call_ids"] = value::array(downlist);
                        }
                    }
                } else cout << "status error??? in !=8\n";
            } else { //full passenger
                elv["elevator_id"] = id;
                if (status == "STOPPED") {
                    // Do something : STOP, UP, DOWN, OPEN

                    //손님중 가장 가까운 목적지가 어디인가?
                    int updest = nearest_upward_dest(passengers, floor);
                    int downdest = nearest_down_dest(passengers, floor);
                    if (updest == floor) { //내릴손님있음
                        elv["command"] = value::string("OPEN");
                    } else if (updest) {
                        if (downdest) {
                            //둘중 가까운곳으로
                            if (updest - floor > floor - downdest) elv["command"] = value::string("DOWN");
                            else elv["command"] = value::string("UP");
                        } else {
                            //updest밖에 없음
                            elv["command"] = value::string("UP");
                        }
                    } else {
                        if (downdest) {
                            elv["command"] = value::string("DOWN");
                        } else {
                            cout << "there is no calls in elv or error\n";
                        }
                    }
                } else if (status == "UPWARD") {
                    // Do something : STOP, UP
                    int updest = nearest_upward_dest(passengers, floor);
                    if (updest == floor) { //내릴손님있음
                        elv["command"] = value::string("STOP");
                    } else if (updest) {
                        //위로가는 손님이 있어야 정상
                        elv["command"] = value::string("UP");
                    } else {
                        cout << "error downward pass in upward elv\n";
                        elv["command"] = value::string("STOP");
                    }

                } else if (status == "DOWNWARD") {

                    // Do something : STOP, DOWN
                    int downdest = nearest_down_dest(passengers, floor);
                    if (downdest == floor) { //내릴손님있음
                        elv["command"] = value::string("STOP");
                    } else if (downdest) {
                        //위로가는 손님이 있어야 정상
                        elv["command"] = value::string("DOWN");
                    } else {
                        cout << "error upward pass in downward elv\n";
                        elv["command"] = value::string("STOP");
                    }
                } else if (status == "OPENED") {
                    // Do something : OPEN, CLOSE, ENTER, EXIT
                    //내릴손님부터 검색
                    vector<value> exit_list = exit_passenger_list(passengers, floor);
                    if (exit_list.size()) {
                        elv["command"] = value::string("EXIT");
                        elv["call_ids"] = value::array(exit_list);
                    } else { //내릴 손님 없음 꽉찬거니까 문닫자
                        elv["command"] = value::string("CLOSE");
                    }
                } else cout << "status error??? in ==8\n";

            }
            elv_cmd.push_back(elv);
        }

//    a["elevator_id"] = 0;
//    a["command"] = value::string("STOP");
//    value b;
//    b["elevator_id"] = 1;
//    b["command"] = value::string("STOP");
//
//    vector<value> cmd;
//    cmd.push_back(a);
//    cmd.push_back(b);
        value js;
        js["commands"] = value::array(elv_cmd);
        my_post("/action", js, token);

    }
    return 0;
}