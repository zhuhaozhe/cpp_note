#include "rw_lock.h"
#include <string>
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>

zone::read_write_mutex rwmutex;

void read_function(std::string & str) {
    zone::unique_read_lock<zone::read_write_mutex> lock( rwmutex );
    std::cout << "read_f: str: " << str << std::endl;
}

void write_function(std::string & str) {
    zone::unique_write_lock<zone::read_write_mutex> lock( rwmutex );
    str += "1";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "write_f: str: " << str << std::endl;
}

int main(){
    std::string str = "";
    std::thread t1(write_function, std::ref(str));
    std::thread t2(write_function, std::ref(str));
    std::thread t3(write_function, std::ref(str));
    // std::thread t3(read_function, std::ref(str));
    t1.join();
    t2.join();
    t3.join();
    return 0;
}