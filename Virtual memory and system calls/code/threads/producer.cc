#include "producer.h"
#include<stdio.h>

Producer :: Producer(int iid)
{
    id = iid;
}

Producer :: ~Producer(){
    printf(" destroying producer %d\n",id);
}

void Producer :: foo(void *)
{
     printf("OK ");
     printf("OK ");
}

void Producer :: test()
{
    Thread * t = new Thread("testing ");
    void * v;
    //t->Fork(foo,v);
}

void Producer :: produ(int i)
{
    printf("producer %d produced product %d\n", id, i);

}


















