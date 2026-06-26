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

#ifndef SIMULATION_POOL_H
#define SIMULATION_POOL_H

#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>

#include "open_char.h"
#include "Simulation.h"

namespace open_char {

class SimulationPool {

    public:
        SimulationPool(Context *ctx);
        ~SimulationPool();

        void SetNumThreads(size_t num_threads);

        void EnqueueSimulation(Simulation *simulation);
        void Start();
        void WaitDone();

        void PrintStats();

    private:
        Context *ctx_;

        std::vector<Simulation *> simulations_;

        size_t enqueued_;
        size_t dispatched_;
        size_t finished_;

        std::queue<Simulation *> queue_;
        std::vector<std::thread> threads_;
        std::mutex lock_;
        std::condition_variable cv_;
        bool stop_;
        size_t num_threads_;
};

};

#endif
