#include "BlockingQueue.h"
#pragma once

//Добавление в очередь
template<typename T>
void thread_safe_queue<T>::enqueue(T item){
    //Если ещё не было shutdown
    if(!time_to_die.load()){
        std::unique_lock<std::mutex> lock(q_mutex);
        //Проверяем не полна ли очередь
        //Ждем пока в очереди будет место
        not_full.wait(lock, [this] () {return blocking_queue.size() != capacity;});
        blocking_queue.push(item);
        not_empty.notify_one();
    }
    //Иначе
    else{
        //Запрещаем добавлять в очередь с момента shutdown
        //Лочимся навсегда на CV (обусловлено тем, что в С++ нельзя вручную завершить поток)
        std::unique_lock<std::mutex> lock(q_mutex);
        not_full.wait(lock, [this] () {return !time_to_die.load();});
    }
}

//Удаление элемента из очереди
template<typename T>
void thread_safe_queue<T>::dequeue(T& item){
    //Если ещё не было shutdown
    if(!time_to_die.load()){
        //Проверяем не пуста ли очередь
        //Если пуста. ждем пока что-нибудь в неё положат
        std::unique_lock<std::mutex> lock(q_mutex);
        not_empty.wait(lock, [this] () {return !blocking_queue.empty();});
        item = blocking_queue.front();
        blocking_queue.pop();
        not_full.notify_one();
    }
    //Иначе
    else{
        //Забираем из очереди, то что там уже было на момент shutdown
        //Лочимся навсегда на CV, т.к. больше в очередь писать нельзя
        //(обусловлено тем, что в С++ нельзя вручную завершить поток)
        std::unique_lock<std::mutex> lock(q_mutex);
        not_empty.wait(lock, [this] () {return !blocking_queue.empty();});
        item = blocking_queue.front();
        blocking_queue.pop();
    }
}
//Таблетка с ядом
//Просто поднимает флаг, говорящий, что пора завершаться
template<typename T>
void thread_safe_queue<T>::shutdown(){
    time_to_die.store(true);;
}
