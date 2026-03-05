
#include <cassert>

#include "SimulationPool.h"
#include "Utils.h"

namespace open_char {

SimulationPool::SimulationPool(Context *ctx) :
    ctx_(ctx),
    dispatch_head_(0),
    dispatch_tail_(0),
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

void SimulationPool::PushSimulation(Simulation *simulation)
{
    simulations_.push_back(simulation);
}

void SimulationPool::RunSimulations()
{
    n_batch_simulations_ = simulations_.size() - dispatch_tail_;
    info("Running %d simulations...", n_batch_simulations_);

    for (size_t i = dispatch_tail_; i < simulations_.size(); i++) {
        queue_.push(simulations_[i]);
    }

    dispatch_head_ = simulations_.size();

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
                }

                simulation->Simulate();

                {
                    std::unique_lock<std::mutex> local_lock(lock_);
                    dispatch_tail_++;
                    size_t finished = n_batch_simulations_ - (simulations_.size() - dispatch_tail_);
                    info("Finished: %d / %d", finished, n_batch_simulations_);
                }
            }
        });
    }

    cv_.notify_all();

    {
        std::unique_lock<std::mutex> lock(lock_);
        stop_ = true;
    }

    cv_.notify_all();

    for (auto& thread : threads_) {
        thread.join();
    }

    // Synchronously do the post processing callback so that we don't
    // need to handle locking of the data model.
    assert (dispatch_tail_ == dispatch_head_);

    threads_.clear();
}

}