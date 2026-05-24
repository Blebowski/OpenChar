
#include <cassert>

#include "SimulationPool.h"
#include "Utils.h"

namespace open_char {

SimulationPool::SimulationPool(Context *ctx) :
    ctx_(ctx),
    enqueued_(0),
    dispatched_(0),
    finished_(0),
    stop_(false),
    num_threads_(1)
{}

SimulationPool::~SimulationPool()
{
    for (auto & sim : simulations_) {
        delete sim;
    }
}

void SimulationPool::SetNumThreads(size_t num_threads)
{
    num_threads_ = num_threads;
}

void SimulationPool::Start()
{
    for (size_t i = 0; i < num_threads_; i++) {
        threads_.emplace_back([this] {
            while (true) {
                Simulation *simulation;
                {
                    std::unique_lock<std::mutex> local_lock(lock_);

                    cv_.wait(local_lock, [this] {
                        return !queue_.empty() || stop_;
                    });

                    if (stop_ && queue_.empty()) {
                        return;
                    }

                    simulation = queue_.front();
                    queue_.pop();
                    dispatched_++;
                }

                simulation->Simulate();

                {
                    std::unique_lock<std::mutex> local_lock(lock_);
                    finished_++;
                }
            }
        });
    }

    cv_.notify_all();
}

void SimulationPool::WaitDone()
{
    cv_.notify_all();
    {
        std::unique_lock<std::mutex> lock(lock_);
        stop_ = true;
    }

    for (auto& thread : threads_) {
        thread.join();
    }

    threads_.clear();
}

void SimulationPool::EnqueueSimulation(Simulation *simulation)
{
    enqueued_++;
    simulations_.push_back(simulation);
    {
        std::unique_lock<std::mutex> local_lock(lock_);
        queue_.push(simulation);
    }
    cv_.notify_all();
}

void SimulationPool::PrintStats()
{
    info("Simulation Pool:");
    info("      Enqueued:  %d", enqueued_);
    info("      Dispatched:  %d", dispatched_);
    info("      Finished:  %d", finished_);
}

}