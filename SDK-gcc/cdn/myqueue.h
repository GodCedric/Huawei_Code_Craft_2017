#ifndef _ARRAY_QUEUE_H
#define _ARRAY_QUEUE_H

#include <iostream>
using namespace std;

template<typename T, int size = 0>
class Queue{
public:
    Queue();

    bool is_empty()const;

    void enqueue(const T&);
    T dequeue();

private:
    T storge[size];
    int first;
    int last;
};

template<typename T, int size>
Queue<T, size>::Queue()
{
    first = last = -1;
}

template<typename T, int size>
bool Queue<T, size>::is_empty()const
{
    return first == -1;
}


template<typename T, int size>
void Queue<T, size>::enqueue(const T& elem)
{
    //if(!is_full()){
        if(last == -1 || last == size -1){
            storge[0] = elem;
            last = 0;
            if(first == -1)
                first = 0;
        }
        else storge[++last] = elem;
    //}
    //else{
        //cout << "Queue full." << endl;
        //exit(EXIT_FAILURE);
    //}
}

template<typename T, int size>
T Queue<T, size>::dequeue()
{
    if(is_empty()){
        cout << "Queue empty." << endl;
        exit(EXIT_FAILURE);
    }

    T tmp;

    tmp = storge[first];
    if(first == last)
        last = first = -1;
    else if(first == size - 1)
        first = 0;
    else ++first;

    return tmp;
}

#endif
