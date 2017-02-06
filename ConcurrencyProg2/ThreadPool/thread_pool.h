#include <iostream>
#include <thread>
#include <future>
#include "BlockingQueue.hpp"

//Пул потоков
template<typename T>
class thread_pool{
public:
    //Конструктор для фиксированного количества потоков(количество ядер)
    thread_pool() : thread_pool(fixed_amount_of_workers()) {}
    //Конструктор от переданного количества потоков
    //Превышение количества ядер приводит к замедлению работы
    //(Больше потоков - дольше работает)
    thread_pool(size_t threads_amount);
    //Конструктор копирования запрещен
    thread_pool(const thread_pool<T>& rhs) = delete;
    //Передаем задачу на исполнение потокам
    std::shared_ptr<std::future<T>> submit_task(std::function<T()> func);

    //Деструктор, корректно завершающий работу пула
    ~thread_pool();
private:
    //Количество потоков в пуле
    size_t threads_amount;

    //Вектор потоков исполнителей
    std::vector<std::thread> workers;

    //Класс для выполняемых задач
    friend class async_task{
    public:
        //По умолчанию генерирует задания с полями = nullptr
        //эквивалентно завершающему заданию
        async_task() : result(nullptr), task(nullptr) {}
        //Конструктор от указателя на promise и функции(задания)
		async_task(std::shared_ptr<std::promise<T> > r, std::function<T()> t) : result(r), task(t) {}
    private:
        //promise в котором будет результат из future
		std::shared_ptr<std::promise<T> > result;
		//Функция - задание
		std::function<T()> task;
	};
    //Блокирующая очередь для заданий
    thread_safe_queue<async_task> blocking_queue;

    //Количество ядер (=количеству потоков)
    int fixed_amount_of_workers();
    //Кладем в очередь задания, указывающие на то, что пора завершаться
    void shutdown_signal();
    //Завершить все потоки
    void join_workers();
};
