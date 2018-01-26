// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "producer.h"
#include "synch.h"
#include "thread.h"
#include "consumer.h"
#define top 100
//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------
List<int> * buffer;
Lock * lock;
int * hsize;
int * csize;
Condition * prodc;
Condition * conc;
static void  produce(void * v)
{
    Producer * p = (Producer *)v;
    int rand=20000;
    while(true){
        rand=20000;
        lock->Acquire();
        if( (*csize) == 1001 )
        {
            printf(" end of %s\n", currentThread->getName());
            conc->Broadcast(lock);
            lock->Release();
            return ;
        }
        while( *hsize == 50)
        {

            conc->Signal(lock);
            prodc->Wait(lock);

        }
        *csize=(*csize)+1;
        int k=*csize;
        int m=(*hsize);
        m+=1;
        *hsize=m;
        buffer->Append(*csize);
        //printf("produced %d by producer %d\n",*csize,*id);
        p->produ(k);

        while(rand>10000) rand--;

        conc->Signal(lock);
        lock->Release();

        while(rand)rand--;

    }

}

static void consume(void * v)
{
    Consumer *c = (Consumer *) v;
    int rand;
    while(true)
    {
        rand=20000;
        lock->Acquire();
        if(*csize==1001 && *hsize==0)
        {
            printf(" No more product for consumer %s\n",currentThread->getName());
            lock->Release();

            return ;
        }
        while( *hsize==0)
        {
            prodc->Signal(lock);
            conc->Wait(lock);
        }
        int j=( int )buffer->Remove();
        *hsize= *hsize -1;
        //printf("consumer %d consumed product %d\n",*id, j);
        c->consu(j);

        while(rand>10000) rand--;

        prodc->Signal(lock);
        lock->Release();

        while(rand--) ;
    }
}

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");
    //initializing all global strucher s
    lock=new Lock("Lock-->");
    buffer = new List<int>;
    int  counter=0, siz=0;
    csize= new int;
    hsize= new int;
    *csize=0;
    *hsize=0;
    conc = new Condition("conc");
    prodc = new Condition("prodc");


    // producer and consumers
    Producer* p1 = new Producer(1);
    Thread * t1 = new Thread("Producer 1 thread ");
    int  * v;
    t1->Fork(produce,p1);

    Producer  * p2 = new Producer(2);
    Thread * t2 = new Thread("producer 2 thread ");
    t2->Fork(produce,p2);

    Consumer * c1= new Consumer(1);
    Thread * ct1 = new Thread("consu thread 1");
    ct1->Fork(consume, c1);

    Consumer * c2 = new Consumer(2);
    Thread * ct2 = new Thread("consu thread 2");
    ct2->Fork(consume,c2);
    //t->Fork(SimpleThread, 1);
    //SimpleThread(0);
}

