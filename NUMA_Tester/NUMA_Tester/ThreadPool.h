#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <windows.h>

// Thread Pool Class to allow Threads per NUMA Node
// Based on the Affinity of each node
class ThreadPool {
public:
    ThreadPool(size_t num_threads, DWORD_PTR affinity_mask) {

        // Create the threads one per Core in the Node
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([this, affinity_mask, i]() {

                // Set the affinity mask for this thread - Any Core in the Node
                HANDLE thread = GetCurrentThread();
                SetThreadAffinityMask(thread, affinity_mask);

                // Thread Loop
                while (true) {
                    // Get the next task
                    std::function<void()> task;
                    {
                        // Lock the mutex
                        std::unique_lock<std::mutex> lock(mutex);
                        // Wait for a task if there are no tasks and the pool is not stopped
                        condition.wait(lock, [this] { return !tasks.empty() || stop; });
                        // Exit the thread if the pool is stopped and there are no tasks
                        if (stop && tasks.empty()) return;
                        // Get the next task
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    // Execute the Task
                    task();
                    {
                        // Lock the mutex and decrement the number of tasks in progress and notify all if all tasks are done
                        std::unique_lock<std::mutex> lock(mutex);
                        --tasks_in_progress;
                        if (tasks_in_progress == 0) {
                            all_tasks_done.notify_all();
                        }
                    }
                }
                });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }
        // Notify all threads to stop
        condition.notify_all();
        // Wait for all threads to finish
        for (std::thread& thread : threads) {
            thread.join();
        }
    }

    // Enqueue a task
    template <class F>
    void enqueue(F&& f) {
        {
            // Lock the mutex and increment the number of tasks in progress and add the task to the queue
            std::unique_lock<std::mutex> lock(mutex);
            tasks.emplace(std::forward<F>(f));
            ++tasks_in_progress;
        }
        condition.notify_one();
    }

    // Wait for all tasks to finish
    void wait_for_all() {
        std::unique_lock<std::mutex> lock(mutex);
        // Wait for all tasks to finish by waiting for the number of tasks in progress to be 0
        all_tasks_done.wait(lock, [this] { return tasks_in_progress == 0; });
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    std::condition_variable all_tasks_done;
    bool stop = false;
    size_t tasks_in_progress = 0;
};

#endif // THREAD_POOL_H

