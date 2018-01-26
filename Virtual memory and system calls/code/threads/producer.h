#ifndef PRODUCER_H
#define PRODUCER_H

#include "list.h"
#include "synch.h"
class Producer{

public :
    int id;
    Producer(int iid);
    ~Producer();
    void foo( void *);
    void test();
    void produ(int i);
};
#endif
