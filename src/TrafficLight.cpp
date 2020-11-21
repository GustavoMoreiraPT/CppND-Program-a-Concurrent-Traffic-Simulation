#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> u_lock(_mutex);

    _condition.wait(u_lock, [this] {return !_queue.empty(); });

    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> u_lock(_mutex);

    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true){
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto currentLight = _messageQueue->receive();

        if(currentLight == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
   threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
// https://www.pluralsight.com/blog/software-development/how-to-measure-execution-time-intervals-in-c-- 
// How to calculate elapsed time between loop cycles.
void TrafficLight::cycleThroughPhases()
{

    float minSeconds = 4.0f;
    float maxSeconds = 6.0f;
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = maxSeconds - minSeconds;
    float r = random * diff;
    double cycleDuration = minSeconds + r;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    while(true){
        //measure the time between two loop cycles
        auto currentTime = std::chrono::high_resolution_clock::now();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::chrono::duration<double> elapsed = currentTime - lastCycleTime;

        if(elapsed >= cycleDuration){
            
            if(_currentPhase == TrafficLightPhase::red){
                _currentPhase = TrafficLightPhase::green;
            }else{
                _currentPhase = TrafficLightPhase::red;
            }
        }

        _messageQueue.send(std::move(_currentPhase));

        lastCycleTime = std::chrono::high_resolution_clock::now();
    } 
}