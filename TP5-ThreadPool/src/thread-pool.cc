/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
#include <stdexcept>
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].available = true;
        wts[i].ts = thread([this, i] { this->worker(i); });
    }
    
    dt = thread([this] { this->dispatcher(); });
}


void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (!thunk) throw invalid_argument("Cannot schedule nullptr function");

    {
        std::lock_guard<std::mutex> lock(queueLock);
        if (done) throw runtime_error("Cannot schedule on destroyed ThreadPool");

        taskQueue.push(thunk);
        ++pendingTasks;
    }

    dispatcher_cv.notify_one();
}

void ThreadPool::wait() {
    unique_lock<mutex> lk(wait_mutex);
    wait_cv.wait(lk, [this] { return pendingTasks.load() == 0; });
}

ThreadPool::~ThreadPool() {
    wait();
    {
        lock_guard<mutex> lock(queueLock);
        done = true;
    }

    dispatcher_cv.notify_all();
    for (auto& w : wts) w.sem.signal();

    if (dt.joinable()) dt.join();

    for (auto& w : wts) if (w.ts.joinable()) w.ts.join();
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> task;
        int worker_id = -1;
        {
            unique_lock<mutex> lock(queueLock);
            dispatcher_cv.wait(lock, [this] {return done || !taskQueue.empty();});
            if (done && taskQueue.empty()) break;

            // Buscar worker disponible
            while (true) {
                if (done) return;
                for (size_t i = 0; i < wts.size(); ++i) {
                    if (wts[i].available.load()) {
                        worker_id = i;
                        break;
                    }
                }

                if (worker_id != -1 && !taskQueue.empty()) break;

                if (done && taskQueue.empty()) return;

                dispatcher_cv.wait(lock);
            }

            if (done && taskQueue.empty()) return;

            // Asignar tarea
            task = taskQueue.front();
            taskQueue.pop();
            wts[worker_id].available = false;
            {
                lock_guard<mutex> thunk_lock(wts[worker_id].thunk_mutex);
                wts[worker_id].thunk = task;
            }
        }

        wts[worker_id].sem.signal();
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        wts[id].sem.wait();
        if (done) break;

        function<void(void)> mytask;
        {
            lock_guard<mutex> thunk_lock(wts[id].thunk_mutex);
            mytask = std::move(wts[id].thunk);
            wts[id].thunk = nullptr;
        }

        if (mytask) {
            try {
                mytask();
            } catch (...) {}
        }

        size_t remain = --pendingTasks;
        if (remain == 0) {
            unique_lock<mutex> lk(wait_mutex);
            wait_cv.notify_all();
        }

        wts[id].available = true;
        dispatcher_cv.notify_all();
    }
}
