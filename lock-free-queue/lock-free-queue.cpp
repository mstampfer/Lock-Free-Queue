#pragma once
#include <atomic>
#include <iostream>

template <typename T>
class LockFreeQueue 
{
private:
    struct Node 
    {
        Node(T val) : value(val), next(nullptr) { }
        T value;
        Node* next;
    };
    Node* first;                            // for producer only
    std::atomic<Node*> divider, last;         // shared
public:
    LockFreeQueue() 
    {
        first = divider = last = new Node(T());           // add dummy separator
    }
    ~LockFreeQueue() 
    {
        while (first != nullptr) 
        {   
            // release the list
            Node* tmp = first;
            first = tmp->next;
            delete tmp;
        }
    }

    template<typename T>
    void Produce(const T& t) 
    {
        last.load()->next = new Node(t);    // add the new item
        last = last.load()->next;      // publish it
        while (first != divider) 
        { 
            // trim unused nodes
            Node* tmp = first;
            first = first->next;
            delete tmp;
        }
    }

    template<typename T>
    bool Consume(T& result) 
    {
        if (divider.load() != last) 
        {         
            // if queue is nonempty
            result = divider.load()->next->value;  // C: copy it back
            divider = divider.load()->next;   // D: publish that we took it
            return true;              // and report success
        }
        return false;               // else report empty
    }
};

int main()
{
    LockFreeQueue<int> q;
    q.Produce(1);
    int res;
    q.Consume(res);
    std::cout << res << std::endl;
}