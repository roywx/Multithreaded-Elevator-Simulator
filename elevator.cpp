// Elevator simulator program with room for improvement
#include <iostream>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
 

const int MAX_FLOORS = 9;
const int NUM_ELEVATORS = 3;

enum Direction {Up ,Down, Idle}; 

struct Request{ 
    Direction goal_dir;
    int target_floor;
    int source_floor;
};

struct Elevator{
private:
    static unsigned int num_elevators;
public:
    unsigned int id;
    unsigned int curr_floor;
    Direction dir;
    std::deque<int> requested_floors;

    Elevator() : curr_floor(0), dir(Direction::Idle){
        id = num_elevators++;
    }
};

unsigned int Elevator::num_elevators = 0;
std::vector<Elevator> all_elevators;

std::unordered_map<int, boost::mutex> e_locks;
boost::mutex request_mtx;

boost::condition_variable handler_cv;
std::unordered_map<int, boost::condition_variable> elevator_cvs;

// FIFO request servicing 
std::queue<Request> floor_requests;

void add_request(int source, int target){
    Request new_r{(target > source ? Up : Down), target, source};
    {
        boost::unique_lock<boost::mutex> lock(request_mtx);
        floor_requests.push(new_r);
        handler_cv.notify_one();
    }
}
void elevator_thread(Elevator& e){
    while(true){
        boost::unique_lock<boost::mutex> e_lock(e_locks[e.id]);
        while(e.requested_floors.empty()){
            e.dir = Direction::Idle;
            elevator_cvs[e.id].wait(e_lock);
        }

        int next_floor = e.requested_floors.front();

        e.requested_floors.pop_front();
        
        /* This functionality should be improved upon:
        once the elevator decides on a floor, it will always go to that floor,
        regardless of whether it could stop at an intermediate floor. */

        // Update current floor early to prevent the elevator from being assigned a request in the opposite direction
        e.curr_floor = next_floor; 

        e_lock.unlock();
        
        std::cout << "Elevator " << e.id << " going to floor: " << next_floor << std::endl;
        boost::this_thread::sleep_for(boost::chrono::seconds(5));  // simulate movement through sleep
    
        std::cout << "Elevator " << e.id << " reached floor: " << next_floor << std::endl;
    }
}

void request_handler(){
    while(true){
        boost::unique_lock<boost::mutex> rq_lock(request_mtx);
        while(floor_requests.empty()){
            handler_cv.wait(rq_lock);
        }
        Request next_r = floor_requests.front();
        floor_requests.pop();
        rq_lock.unlock();
        bool assigned = false; 
        
        // check if any elevator are already heading in the direction of a request
        for(Elevator& e : all_elevators){
            boost::unique_lock<boost::mutex> elock(e_locks[e.id]);
            if(e.dir == next_r.goal_dir){
                if((e.curr_floor >= next_r.source_floor && e.dir == Direction::Down) || 
                (e.curr_floor <= next_r.source_floor && e.dir == Direction::Up)){
                     if (std::find(e.requested_floors.begin(), e.requested_floors.end(), next_r.source_floor) == e.requested_floors.end()){
                        e.requested_floors.push_back(next_r.source_floor);
                        std::cout << "Request from floor " << next_r.source_floor << " assigned to elevator " << e.id << "\n";
                    }else{
                        std::cout << "Request from floor " << next_r.source_floor << " already assigned to elevator " << e.id << "\n";
                    }

                    if (std::find(e.requested_floors.begin(), e.requested_floors.end(), next_r.target_floor) == e.requested_floors.end()){
                        e.requested_floors.push_back(next_r.target_floor);
                        std::cout << "Request to floor " << next_r.target_floor << " assigned to elevator " << e.id << "\n";
                    }else{
                        std::cout << "Request to floor " << next_r.target_floor << " already assigned to elevator " << e.id << "\n";
                    }
                   
                    if (e.dir == Up){
                        std::sort(e.requested_floors.begin(), e.requested_floors.end());
                    }
                    else if (e.dir == Down){
                        std::sort(e.requested_floors.begin(), e.requested_floors.end(), std::greater<int>());
                    }
                    assigned = true;
                    break;
                }
            }
        }

        // if no elevator was heading in the direction of a request, give to idle elevator
        if(!assigned){
            for(Elevator& e : all_elevators){
                boost::unique_lock<boost::mutex> elock(e_locks[e.id]);
                if(e.dir == Direction::Idle){
                    assert(std::find(e.requested_floors.begin(), e.requested_floors.end(), next_r.source_floor) == e.requested_floors.end());
                    assert(std::find(e.requested_floors.begin(), e.requested_floors.end(), next_r.target_floor) == e.requested_floors.end());
                    e.requested_floors.push_back(next_r.source_floor);
                    e.requested_floors.push_back(next_r.target_floor);
                    // Direction is set to match the request, could be improved for smarter scheduling
                    e.dir = next_r.goal_dir;
                    std::cout << "Request from floor " << next_r.source_floor << " to " <<  next_r.target_floor << " assigned to idle elevator " << e.id << "\n";
                    
                     if (e.dir == Up){
                        std::sort(e.requested_floors.begin(), e.requested_floors.end());
                    }
                    else if (e.dir == Down){
                        std::sort(e.requested_floors.begin(), e.requested_floors.end(), std::greater<int>());
                    }
                    assigned = true;
                    elevator_cvs[e.id].notify_one();
                    break;
                }
            }
        }
       
        // Requeuing could be made more efficient, if all elevators are busy then this causes busy waiting
        if(!assigned){
            boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            boost::unique_lock<boost::mutex> lock(request_mtx);
            floor_requests.push(next_r);
        }
    }
    
}

int main(int argc, char* argv[]){
    // initialize threads
   for (int i = 0; i < NUM_ELEVATORS; ++i) {
        all_elevators.emplace_back(); 
        e_locks[i];
        elevator_cvs[i];
    }

    boost::thread handler_thread(&request_handler);

    std::vector<boost::thread> elevator_threads;
    for (int i = 0; i < NUM_ELEVATORS; ++i) {
        elevator_threads.emplace_back(boost::bind(&elevator_thread, boost::ref(all_elevators[i])));
    }

    // get user requests
    std::string input;
    while(std::getline(std::cin, input)){
        int source, target;    
        std::istringstream iss(input);

        if(iss >> source >> target){
            if(source >= 0 && source < MAX_FLOORS && target >= 0 && target < MAX_FLOORS){
                std::cout << "Request from floor " << source << " to floor " << target << " added\n";
                add_request(source, target);
            }else{
                std::cout << "Invalid input: Out of range\n";
            }
        }else{
            std::cout << "Invalid input. Expected format: <source_floor> <target_floor>\n";
        }
    }
    return 0;
}