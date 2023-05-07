#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std;
using namespace std::chrono;


int PassengerNumber;
int CarNumber;
double lambda_p;
double lambda_c;
int k;

// Mutexes and condition variables
mutex car_lock;
mutex passenger_lock;
condition_variable CarBusy;
condition_variable PassengerWaiting;
condition_variable CarFree;

// State variables
int FreeCarCount;
queue<int> WaitingVector;
vector<bool> passenger_riding;
vector<int> RidesDonePerPassenger;

default_random_engine generator;
exponential_distribution<double> mean(1.0);
ofstream log_file("output.txt");

double WaitTimeExp(double lambda)
{
    exponential_distribution<double> dist(lambda);
    return dist(generator);
}

// Passenger thread function
void passenger_func(int id)
{
    for (int i = 0; i < k; i++)
    {
        
        int WaitTime = WaitTimeExp(lambda_p);
        CurrThread::sleep_for(microseconds(int(WaitTime * 1000)));

        unique_lock<mutex> lock(passenger_lock);
        PassengerWaiting.wait(lock, [id]
                               { return FreeCarCount > 0 && !passenger_riding[id]; });

 
        WaitingVector.push(id);
        FreeCarCount--;
        passenger_riding[id] = true;
        log_file << "Passenger " << id << " made a ride request at " << time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count() << endl;
        CarBusy.notify_one();

        CarFree.wait(lock, [id]
                                { return !passenger_riding[id]; });
        FreeCarCount++;
        passenger_riding[id] = false;
        log_file << "Passenger " << id << " finished riding at " << time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count() << endl;
    }
}

// Car thread function
void car_func(int id)
{
    while (true)
    {
        
        unique_lock<mutex> lock(car_lock);
        CarBusy.wait(lock, []
                       { return !WaitingVector.empty(); });

       
        int passenger_id = WaitingVector.front();
        WaitingVector.pop();
        RidesDonePerPassenger[passenger_id]++;
        log_file << "Car " << id << " accepts passenger " << passenger_id << "'s request at " << time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count() << endl;

        
        double ride_time = mean(generator);
        CurrThread::sleep_for(microseconds(int(ride_time * 1000000)));
        

        
        passenger_riding[passenger_id] = false;
        log_file << "Car " << id << " finishes riding Passenger " << passenger_id << " at " << time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count() << endl;
        CarFree.notify_one();
    }
}

int main()
{
  
    ifstream input_file("inp-params.txt");
    input_file >> PassengerNumber >> CarNumber >> lambda_p >> lambda_c >> k;

    FreeCarCount = CarNumber;
    passenger_riding = vector<bool>(PassengerNumber, false);
    RidesDonePerPassenger = vector<int>(PassengerNumber, 0);

  
    const int num_trials = 10;
    const int CarNumber = 25;
    const int k = 5;

    // Vary the number of passengers
    vector<int> PassengerNumber_vec = {10, 15, 20, 25, 30, 35, 40, 45, 50};
    vector<double> AvgTimeVector;

    for (int i = 0; i < PassengerNumber_vec.size(); i++)
    {
        int PassengerNumber = PassengerNumber_vec[i];
        double TotalTime = 0.0;

        // Run multiple trials and record the average time taken by passengers
        for (int j = 0; j < num_trials; j++)
        {
            // Create the passenger threads
            vector<thread> passenger_threads;
            for (int i = 0; i < PassengerNumber; i++)
            {
                passenger_threads.push_back(thread(passenger_func, i));
            }

            // Create the car threads
            vector<thread> car_threads;
            for (int i = 0; i < CarNumber; i++)
            {
                car_threads.push_back(thread(car_func, i));
            }

            // Wait for the passenger threads to finish
            for (auto &thread : passenger_threads)
            {
                thread.join();
            }

            // Tell the car threads to exit
            for (int i = 0; i < CarNumber; i++)
            {
                CarBusy.notify_one();
            }

            // Wait for the car threads to finish
            for (auto &thread : car_threads)
            {
                thread.join();
            }

            // Record the time taken by passengers
            double time_sum = 0.0;
            for (int i = 0; i < PassengerNumber; i++)
            {
                time_sum += RidesDonePerPassenger[i] * mean(generator);
            }
            TotalTime += time_sum / PassengerNumber;
        }

        // Record the average time taken by passengers across trials
        double avg_time = TotalTime / num_trials;
        AvgTimeVector.push_back(avg_time);
    }

    // Plot the results
    Gnuplot gp;
    gp << "set terminal pngcairo\n";
    gp << "set output 'plot1.png'\n";
    gp << "set xlabel 'Number of Passengers'\n";
    gp << "set ylabel 'Average Time Taken (s)'\n";
    gp << "plot '-' with lines title 'Average Time Taken'\n";
    for (int i = 0; i < PassengerNumber_vec.size(); i++)
    {
        gp << PassengerNumber_vec[i] << " " << AvgTimeVector[i] << "\n";
    }
    gp << "e\n";

    return 0;
}
