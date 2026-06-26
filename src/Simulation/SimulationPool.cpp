////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

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
    {
        std::unique_lock<std::mutex> lock(lock_);
        stop_ = true;
    }
    cv_.notify_all();

    for (auto& thread : threads_) {
        thread.join();
    }

    threads_.clear();

    stop_ = false;
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