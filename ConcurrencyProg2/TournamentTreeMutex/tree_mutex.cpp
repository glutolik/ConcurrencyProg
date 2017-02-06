#include "tree_mutex.h"
#include <assert.h>
#pragma once

//вычисляет наименьшую степень двойки большую n/2
int tree_mutex::to_pow(int n){
    assert(n != 0);
    int pow = 1;
    while(pow < (n + 1) / 2){
        pow = pow * 2;
    }
    return pow;
}


//Захватывает мьютекс
void PetersonMutex::lock (size_t index){
    assert(index < 2);
    want[index].store(true);
    victim.store(index);
    while(want[1 - index].load() && victim.load()  == index){
        std::this_thread::yield();
    }
}

//Освобождает мьютекс
void PetersonMutex::unlock(size_t index){
    assert(index < 2);
    want[index].store(false);
}

//Захватывает мьютекс
void tree_mutex::lock(size_t index){
    assert(index < num);
    size_t id = tree.size() + index;
    while(id != 0){
        tree[(id - 1) / 2].lock((id + 1) & 1);
        id = (id - 1) / 2;
    }
}

//Освождает мьютекс
void tree_mutex::unlock(size_t index) {
    assert(index < num);
    size_t depth = 0;
    while ((1 << depth) < num) depth++; //глубина дерева
    size_t mutex_id = 0;
    for (int i = depth - 1; i >= 0; i--) {
        tree[mutex_id].unlock((index >> i) & 1); //смотрим из какого потомка поток попал в данный мьютекс
        mutex_id = mutex_id * 2 + 1 + ((index >> i) & 1); //вычисляем id предыдущего мьютекса, через который прошел данный поток
    }
}
