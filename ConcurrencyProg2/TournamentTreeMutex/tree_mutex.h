#include <thread>
#include <iostream>
#include <vector>
#include <atomic>
#include <array>

#pragma once

//Мьютекс Петерсона. Можно использовать только 2-мя потоками
class PetersonMutex{
public:
    //Конструктор по умолчанию
    PetersonMutex() {
        want[0].store(false);
        want[1].store(false);
        victim.store(0);
    }
    //Захватывает мьютекс
    void lock (size_t index);
    //Освобождает мьютекс
    void unlock(size_t index);

private:
    std::array<std::atomic<bool>, 2> want; // флаги (true == поток с соответствующим номером хочет захватить мьютекс)
    std::atomic<size_t> victim; //жертва
};

class tree_mutex{
public:
    tree_mutex(size_t n):
        num(n),
        tree(2 * to_pow(n) - 1) //размер дерева
        {}

    void lock(size_t index);
    void unlock(size_t index);

private:
    size_t num; //количество потоков
    std::vector<PetersonMutex> tree; //дерево на мьютексах Петрсона


    int to_pow(int n); //вычисляет наименьшую степень двойки большую n/2
};
