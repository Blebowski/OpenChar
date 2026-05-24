

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
