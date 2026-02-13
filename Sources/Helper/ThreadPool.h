#pragma once 

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

// Class template taken from: https://www.geeksforgeeks.org/cpp/thread-pool-in-cpp/
class ThreadPool {
public:

    ThreadPool(size_t num_threads = thread::hardware_concurrency())
    {

        // Creating worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    function<void()> task;
                    {
                        // Locking the queue so that data can be shared safely
                        unique_lock<mutex> lock(queue_mutex_);

                        // Waiting until there is a task to execute or the pool is stopped
                        cv_.wait(lock, [this] {
                            return !tasks_.empty() || stop_;
                            });

                        // exit the thread in case the pool is stopped and there are no tasks
                        if (stop_ && tasks_.empty()) {
                            return;
                        }

                        // Get the next task from the queue
                        task = move(tasks_.front());
                        tasks_.pop();
						activeTasks++;
                    }

                    task();

                    {
                        std::lock_guard<std::mutex> lock(queue_mutex_);
                        activeTasks--;
                    }
                    cv_.notify_all();
                }
                }
            );
        }

    }

    // Destructor to stop the thread pool
    ~ThreadPool()
    {
        {
            // Lock the queue to update the stop flag safely
            unique_lock<mutex> lock(queue_mutex_);
            stop_ = true;
        }

        // Notify all threads
        cv_.notify_all();

        // Joining all worker threads to ensure they have
        // completed their tasks
        for (auto& thread : workers_) {
            thread.join();
        }
    }

    // Enqueue task for execution by the thread pool
    void enqueue(function<void()> task)
    {
        {
            unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(move(task));
        }
        cv_.notify_one();
    }

    void wait()
    {
        unique_lock<mutex> lock(queue_mutex_);
        cv_.wait(lock, [this] { return tasks_.empty() && activeTasks == 0; });
	}

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;


    std::mutex queue_mutex_;
    std::condition_variable cv_;
	std::atomic<int> activeTasks = 0; // count of tasks currently being executed
	bool stop_ = false;

};

