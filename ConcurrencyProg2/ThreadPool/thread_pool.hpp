#include "thread_pool.h"
#pragma once

template<typename T>
thread_pool<T>::thread_pool(size_t threads_amount) : threads_amount(threads_amount){
    auto function_to_execute = [this]() -> void
	{

        //Создаем ново задание
		async_task next_async_task;
		//вытаскиваем задание из очереди и кладем его в next_async_task
		blocking_queue.dequeue(next_async_task);
        //пока не увидим завершающее задание, вытаскиваем задания из очереди
		while (next_async_task.task != nullptr)
		{
            //Передаем найденный результат потребителю
			next_async_task.result->set_value(next_async_task.task());
			//вытаскиваем следующее задание из очереди
			blocking_queue.dequeue(next_async_task);
		}
	};
    //Запускаем worker'ов на исполнение задач
	for (int i = 0; i < threads_amount; ++i)
		workers.push_back(std::thread(function_to_execute));
}

template<typename T>
void thread_pool<T>::shutdown_signal(){
    //Конструктор по умолчанию async_tassk гунерирует завершающие задания
    //кладем их в оередь столько, сколько есть worker'ов
	for (int i = 0; i < threads_amount; ++i)
		blocking_queue.enqueue(async_task());
}

//Ожидаем завершения worker'ов
template<typename T>
void thread_pool<T>::join_workers(){
    for (int i = 0; i < workers.size(); ++i)
		if (workers[i].joinable())
			workers[i].join();
}

//Деструкор отправляет в очередь завершающие задания и жде завершения всех воркеров
template<typename T>
thread_pool<T>::~thread_pool(){
    shutdown_signal();
    join_workers();
}

//Находит количество ядер и задает количество потоков worker'ов
template<typename T>
int thread_pool<T>::fixed_amount_of_workers()
{
    //Количество ядер
	size_t cores_num = std::thread::hardware_concurrency(); // = количеству ядер || = 0

	return (cores_num = 0) ? 4 : cores_num; // если 0, то по умолчанию вернем 4
}

//Передает задание на исполнение
template<typename T>
std::shared_ptr<std::future<T>> thread_pool<T>::submit_task(std::function<T()> func){
    //Указатель на promise(result) для worker'а
	auto result_ptr = std::make_shared<std::promise<T> >(std::promise<T>());

	//Указатель на future, в котором будет результат функции
	auto future_ptr = std::make_shared<std::future<T> >(result_ptr->get_future());

	//Кладем задачу, построенную от result_ptr и future_ptr в очередь для дальнейшего её исполнения
	blocking_queue.enqueue(async_task(result_ptr, func));
    //Возвращаем future, в котором когда-нибудь будет лежать результат
	return future_ptr;
}
