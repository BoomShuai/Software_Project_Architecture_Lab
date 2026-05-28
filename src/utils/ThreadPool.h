#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

/**
 * @class ThreadPool
 * @brief Lightweight thread pool for parallel task execution.
 *
 * Manages a fixed number of worker threads that pull tasks from a shared queue.
 * Used primarily for parallelizing the daily settlement process across
 * large inventories.
 *
 * Design decisions:
 * - Fixed thread count (avoids overhead of dynamic thread creation)
 * - FIFO task queue with condition variable signaling
 * - RAII-based lifecycle: threads are joined on destruction
 */
class ThreadPool {
public:
    /**
     * @brief Creates a thread pool with the specified number of workers.
     * @param numThreads Number of worker threads (default: hardware concurrency).
     */
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
        : stop_(false) {
        if (numThreads == 0) numThreads = 2;
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        condition_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                    ++completedTasks_;
                }
            });
        }
    }

    /**
     * @brief Submits a task for execution by the thread pool.
     * @param task A callable to execute.
     * @return A future that will hold the result of the task.
     */
    template<typename F>
    auto submit(F&& task) -> std::future<decltype(task())> {
        using ReturnType = decltype(task());
        auto packagedTask = std::make_shared<std::packaged_task<ReturnType()>>(
            std::forward<F>(task)
        );
        std::future<ReturnType> future = packagedTask->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("Cannot submit to stopped ThreadPool");
            }
            tasks_.emplace([packagedTask]() { (*packagedTask)(); });
        }
        condition_.notify_one();
        return future;
    }

    /**
     * @brief Returns the number of worker threads.
     */
    size_t numWorkers() const { return workers_.size(); }

    /**
     * @brief Returns the number of completed tasks.
     */
    size_t getCompletedTasks() const { return completedTasks_.load(); }

    /**
     * @brief Destructor. Signals all threads to stop and joins them.
     */
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // Non-copyable, non-movable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_;
    std::atomic<size_t> completedTasks_{0};
};

#endif // THREAD_POOL_H
