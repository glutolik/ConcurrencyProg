#include <vector>
#include <atomic>

const int CACHE_LINE_SIZE = 64; //Размер кэш линии

template <class T>
class spsc_ring_buffer{
public:
    //Конструктор по умолчанию (создается буффер с заданным размером)
    spsc_ring_buffer(size_t size): data(size + 1), head(0), tail(0){}
    //Добавление элемента в буфер (если это возможно - true, иначе - false)
    bool enqueue(T item);
    //Удаление элемента из очереди (если не пуст - true, иначе - false)
    bool dequeue(T& item);

private:
    // В узле хранится объект типа Т + массив байтов char
    class node_t{
	public:
	    node_t(){};
		node_t(T item): elem(item){}
		T get_elem(){return elem;}
	private:
		T elem;
		char str[CACHE_LINE_SIZE]; //Массив батов char, гарантирует, что разные элементы попадают в разные кэш линии
	};

    std::vector<node_t> data; //Данные
    std::atomic<size_t> head; //Голова - откуда извлекаем
    std::atomic<size_t> tail; //Хвост - куда записываем
    size_t get_next_id(size_t current); //Получить указатель на следующий элемент
};

template <class T>
size_t get_next_id(size_t current);{
    return (current + 1) % data.size();
}

template <class T>
bool spsc_ring_buffer<T>::enqueue(T item){
    size_t cur_tail = tail.load(std::memory_order_acquire);
    size_t next_tail = get_next_id(cur_tail);
    if (next_tail == head.load(std::memory_order_acquire))
        return false;
    data[cur_tail] = node_t(item);
    tail.store(next_tail, std::memory_order_release);
    return true;
}

template <class T>
bool spsc_ring_buffer<T>::dequeue(T& item){
    size_t current_head = head.load(std::memory_order_acquire);
    if (current_head == tail.load(std::memory_order_acquire))
        return false;
    item = data[current_head].get_elem();
    head.store(get_next_id(current_head), std::memory_order_release);
    return true;
}


