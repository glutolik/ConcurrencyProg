
template <class T>
class lock_free_queue{
public:
    lock_free_queue(){
        node* new_node = new node();
        head.store(new_node);
        tail.store(new_node);
        for_delete.store(new_node);
    }
    void enqueue(T value);
    bool dequeue(T& value);
    struct node{
        node():next(nullptr){}
        node(T& item):next(nullptr),data(item){}
        std::atomic<node*> next;
        T data;
    };
    ~lock_free_queue (){
        for (node* it = for_delete.load(); it != nullptr;){
            node* tmp = it->next.load();
            delete it;
            it = tmp;
        }
    }
private:
    std::atomic<size_t> count;
    std::atomic<node*> for_delete;
    std::atomic<node*> tail;
    std::atomic<node*> head;
};

template <class T>
void lock_free_queue<T>::enqueue(T value){
    node* pNode = new node(value);
    node* tail_next;
    node* tmp_tail;
    count.fetch_add(1);
    while(true){
        tmp_tail = tail.load();
        tail_next = tmp_tail->next.load();
        if (tmp_tail == tail){
            if (!tail_next){
                if ((tail.load())->next.compare_exchange_strong(tail_next, pNode)){
                    break;
                }
            }else{
                tail.compare_exchange_strong(tmp_tail, tail_next);
            }
        }
    }
    tail.compare_exchange_strong(tmp_tail, pNode);
    count.fetch_sub(1);
}

template <class T>
bool lock_free_queue<T>::dequeue(T& value){
    count.fetch_add(1);
    node* tmp_head;
    node* tmp_tail;
    node* tmp_head_next;
    while(true){
        tmp_head = head.load();
        tmp_tail = tail.load();
        tmp_head_next = tmp_head->next.load();
        if (head == tmp_head){
            if (tmp_head == tmp_tail){
                if (!tmp_head_next) {
                    return false;
                }
                    tail.compare_exchange_strong(tmp_head, tmp_head_next);
            }
            else if (head.compare_exchange_strong(tmp_head, tmp_head_next)){
                value = tmp_head_next->data;
                break;
            }
       }
    }
    if (count.load() == 1){
        node* tmp = for_delete.exchange(tmp_head_next);
        while (tmp != for_delete.load()){
            node * tmp1 = tmp->next.load();
            delete tmp;
            tmp = tmp1;
        }
    }
    count.fetch_sub(1);
    return true;
}
