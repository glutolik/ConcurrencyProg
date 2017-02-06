#include <iostream>
#include <mutex>
#include <vector>
#include <forward_list>
#include <atomic>
#include <algorithm>

template<class T, class Hash = std::hash<T> >
class striped_hash_set
{
public:
    //Конструктор по умолчанию
    striped_hash_set (size_t mutexNum_=1, size_t growthFactor_=2, double loadFactor_=100)
                     : growthFactor(growthFactor_),
                       loadFactor(loadFactor_),
                       locks(mutexNum_),
                       elementsNum(0),
                       table(67 * mutexNum_)
    {}

    //Конструктор копирования и оператор присваивания запрещены
    striped_hash_set(const StripedHashSet &rhs) = delete;
	striped_hash_set &operator=(const StripedHashSet &rhs) = delete;

	//Деструктор по умолчанию
	~striped_hash_set() = default;

	//Добавление элемента (в случае если такой элемент уже имеется, ничего не происходит)
    void add(const T& elem);

    //Удаление элеммента (если такого элемента нет, ничего не происходит)
    void remove(const T& elem);

    //Если данный элемент уже содержится - true, иаче - false
    bool contains(const T& elem) const;

private:

    size_t get_bucket_index(size_t hash_value) const; //Возвращает номер корзины, в которой лежит элемент
    size_t get_stripe_index(size_t hash_value) const; //Возвращает номер stripe, который соответствует элементу

    const size_t growthFactor; //Коэффициент увеличения (по умолчанию = 2)
    const double loadFactor; //Отношение количества элементов к количеству "корзин" в таблице

    mutable std::vector<std::mutex> locks;

    std::atomic<size_t> elementsNum; //Количество элементов в таблице на данный момент

    std::vector<std::forward_list<T> > table; //Сама таблица
};

template<class T, class Hash>
inline void striped_hash_set<T, Hash>::add(const T& elem)
{
    //Если такой элемент уже есть - ничего не делаем
    if (std::find(bucket.begin(), bucket.end(), elem) != bucket.end)
		return;

    size_t hash_value = Hash()(elem);
    std::unique_lock<std::mutex> ul(locks[get_stripe_index(hash_value)]); //Захватываем владение нужным мьютексом
    std::forward_list<T>& bucket = table[get_bucket_index(hash_value)];


    bucket.push_front(elem);
    // Если отноение количества элементов к размеру таблицы превысило loadFactor, то таблицу надо расширять
    if (static_cast<double>(elementsNum.fetch_add(1)) / table.size() >= loadFactor)
    {
        size_t oldSize = table.size();
        ul.unlock();
        std::vector<std::unique_lock<std::mutex> > ulocks;
        ulocks.emplace_back(locks[0]);
        if(oldSize == table.size())
        {
            for(size_t i = 1; i < locks.size(); ++i)
                ulocks.emplace_back(locks[i]);
            auto oldTable = table;
            size_t size = growthFactor * table.size();
            table.clear();
            table.resize(size);
            for(const auto& buck : oldTable)
                for(const auto& elm : buck)
                    table[Hash()(elm) % size].push_front(elm);
        }
    }
}

template<class T, class Hash>
inline void striped_hash_set<T, Hash>::remove(const T& elem)
{
    //Если такого элемента нет, ничего делать не надо
    if (!сontains(elem))
		return;

    size_t hash_value = Hash()(elem);
    std::unique_lock<std::mutex> ul(locks[get_stripe_index(hash_value)]);  //Захватываем владение нужным мьютексом
    std::forward_list<T>& bucket = table[get_bucket_index(hash_value)];
    bucket.remove(elem); //Удаляем элемент
    elementsNum.fetch_sub(1); //Количество элементов уменьшилось на 1
}

template<class T, class Hash>
inline bool striped_hash_set<T, Hash>::contains(const T& elem) const
{
    size_t hash_value = Hash()(elem);
    std::unique_lock<std::mutex> ul(locks[get_stripe_index(hash_value)]); //Захватываем владение нужным мьютексом
    std::forward_list<T>& bucket = table[get_bucket_index(hash_value)]; //Находим нужную корзину
    return std::find(bucket.begin(), bucket.end(), elem) != bucket.end();
}

template<class T, class Hash>
inline size_t striped_hash_set<T, Hash>::get_bucket_index(size_t hash_value) const
{
    return hash_value % table.size();
}

template<class T, class Hash>
inline size_t striped_hash_set<T, Hash>::get_stripe_index(size_t hash_value) const
{
    return hash_value % locks.size();
}
