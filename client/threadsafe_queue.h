#ifndef THREADSAFE_QUEUE
#define THREADSAFE_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class ThreadsafeQueue {
public:

  ThreadsafeQueue() : queue_(), m_(), not_empty_() {}

  void Enqueue(T value) {
    std::lock_guard<std::mutex> lock(m_);
    queue_.push(value);
    not_empty_.notify_one();
  }

  T Dequeue() {
    std::unique_lock<std::mutex> lock(m_);
    while (queue_.empty() && !killed_) {
      not_empty_.wait(lock);
    }

    if (killed_) {
      return T();
    }

    T ret = queue_.front();
    queue_.pop();
    return ret;
  }

  void Kill() {
    killed_ = true;
  }

private:
  std::queue<T> queue_;
  mutable std::mutex m_;
  std::condition_variable not_empty_;
  std::atomic_bool killed_;
};
#endif