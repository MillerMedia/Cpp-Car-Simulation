#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mtx);
    _cond.wait(lck, [this] { return !_queue.empty(); });

    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtx);
    
    std::cout << "Light changed." << std::endl;;
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

/* 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}
*/

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while( true ){
        auto receive_message = _messageQueue.receive();

        if(receive_message == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // @todo add a mutex?

    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread t(&TrafficLight::cycleThroughPhases, this);
    threads.emplace_back(std::move(t));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    // Reference: https://en.cppreference.com/w/cpp/thread/sleep_for
    int while_count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    auto end = 0;
    
    // From: https://stackoverflow.com/a/7560564/975592
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(4, 6); // define the range
    auto cycle_duration = distr(gen);
    int cycle = 0;

    std::string msg = "";
    
    while( true ){
        cycle++;

        if(while_count%2==0){
            auto start = std::chrono::high_resolution_clock::now();
        }

        if(cycle==cycle_duration){
            if(getCurrentPhase() == TrafficLight::TrafficLightPhase::green){
                setCurrentPhase(TrafficLight::TrafficLightPhase::red);
            } else if (getCurrentPhase() == TrafficLight::TrafficLightPhase::red){
                setCurrentPhase(TrafficLight::TrafficLightPhase::green);
            }

            // @todo Send message about light change
            _messageQueue.send(getCurrentPhase());

            // Regenerate cycle duration
            cycle_duration = distr(gen);
            cycle = 0;
        }

        // Not sure how effective modulus is? Could it run slower and slower
        // as the number of loops get higher?
        if(while_count%2!=0){
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end-start;
        }

        while_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}