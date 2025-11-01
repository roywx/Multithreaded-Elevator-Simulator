// Elevator simulator program
#include <iostream>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
 
boost::mutex mtx;

void print_hello(int id){
    boost::unique_lock<boost::mutex> lock(mtx);
    std::cout << "Hello from thread " << id << std::endl;
}

int main(int argc, char* argv[]){
    boost::thread t1(print_hello, 1);
    boost::thread t2(print_hello, 2);

    t1.join();
    t2.join();

    std::cout << "All threads done!" << std::endl;
    return 0;
}