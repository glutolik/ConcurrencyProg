#include <thread>
#include <iostream>
#include <vector>
#include <atomic>
#include <array>

#pragma once

//������� ���������. ����� ������������ ������ 2-�� ��������
class PetersonMutex{
public:
    //����������� �� ���������
    PetersonMutex() {
        want[0].store(false);
        want[1].store(false);
        victim.store(0);
    }
    //����������� �������
    void lock (size_t index);
    //����������� �������
    void unlock(size_t index);

private:
    std::array<std::atomic<bool>, 2> want; // ����� (true == ����� � ��������������� ������� ����� ��������� �������)
    std::atomic<size_t> victim; //������
};

class tree_mutex{
public:
    tree_mutex(size_t n):
        num(n),
        tree(2 * to_pow(n) - 1) //������ ������
        {}

    void lock(size_t index);
    void unlock(size_t index);

private:
    size_t num; //���������� �������
    std::vector<PetersonMutex> tree; //������ �� ��������� ��������


    int to_pow(int n); //��������� ���������� ������� ������ ������� n/2
};
