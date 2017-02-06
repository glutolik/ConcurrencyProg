#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <atomic>

//Потокобезопасная очередь
template<typename T>
class thread_safe_queue{
public:
    //Конструктор по умолчанию
    thread_safe_queue<T>(size_t cap): capacity(cap){
        time_to_die.store(false);
    };
    //Явно указываем, что конструктор копирования запрещен
    thread_safe_queue<T>(const thread_safe_queue<T>& rhs) = delete;
    //Добавление элемента в очередь
    void enqueue(T item);
    //Удаление элемента из очереди
    void dequeue(T& item);
    //Завершение работы очереди
    void shutdown();
private:
    std::queue<T> blocking_queue;
    size_t capacity;
    std::mutex q_mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    //Флаг для завершения работы
    std::atomic<bool> time_to_die;
};
