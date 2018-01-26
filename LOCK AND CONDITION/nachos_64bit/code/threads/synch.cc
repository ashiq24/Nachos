// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List<Thread*>;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
	queue->Append(currentThread);		// so go to sleep
	currentThread->Sleep();
    }
    value--; 					// semaphore available,
						// consume its value

    interrupt->SetLevel(oldLevel);		// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName)
{
	name=debugName;
	islock=false;
	lockingThread=NULL;
	lqueue = new List<Thread*>;

}
Lock::~Lock()
{
	delete lqueue;
}

bool Lock::isHeldByCurrentThread()
{
    if( currentThread==lockingThread) return true;
    else return false;
}
void Lock::Acquire()
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	//printf("tring to aqure lock %s\n",currentThread->getName());
	while(islock)
	{
        //printf("GOT FASLE \n");
		lqueue->Append(currentThread);
		currentThread->Sleep();
	}
	islock=true;
	lockingThread=currentThread;
	//printf("got the lock %s\n", currentThread->getName());
	interrupt->SetLevel(oldLevel);		// re-enable interrupts

}
void Lock::Release(){
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(this->isHeldByCurrentThread())
    {
        //printf("unlocking of lock by %s\n", currentThread->getName());
        Thread * nextThread;
        if(!lqueue->IsEmpty()) nextThread = lqueue->Remove();
        else nextThread = NULL;

        if( nextThread!= NULL)
        {
            scheduler->ReadyToRun(nextThread);
        }
        else
        {
            //printf("no thread to run on lock %s\n", getName());
        }
        islock=false;
        lockingThread= NULL;

    }
    else
    {
        printf("Error release %s, not locked by current lock\n", currentThread->getName());
    }

    interrupt->SetLevel(oldLevel);

}

Condition::Condition(char* debugName) {
    this->name=debugName;
    this->cqueue= new List<Thread*>;

}
Condition::~Condition() {
    delete cqueue;
}
void Condition::Wait(Lock* conditionLock) {
    //printf("condition waiting thread  %s\n", currentThread->getName());
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(conditionLock->isHeldByCurrentThread())
    {
        conditionLock->Release();
        cqueue->Append(currentThread);
        currentThread->Sleep();
        interrupt->SetLevel(oldLevel);
        conditionLock->Acquire();
    }
    else
    {
        interrupt->SetLevel(oldLevel);
    }
    //printf("condition wait finished for  %s\n", currentThread->getName());

 }
void Condition::Signal(Lock* conditionLock) {

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if( conditionLock->isHeldByCurrentThread())
    {
        Thread * nextThread;
        if(!cqueue->IsEmpty()) nextThread = cqueue->Remove();
        else nextThread = NULL;

        if( nextThread != NULL )
        {
            scheduler->ReadyToRun(nextThread);
        }
        else
        {
            //printf("signaled but no one waiting");
        }
    }
    else
    {
        printf("Lock not accired by the calling thread\n");

    }
    interrupt->SetLevel(oldLevel);



}
void Condition::Broadcast(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if( conditionLock->isHeldByCurrentThread())
    {

        while( true )
        {
             Thread * readythread ;
             if(!cqueue->IsEmpty())readythread =  cqueue->Remove();
             else readythread = NULL;

             if( readythread != NULL)
             {
                scheduler->ReadyToRun(readythread);

             }else
             {
                break;
             }
        }

    }
    else
    {
        printf("Lock not accired by the calling thread\n");

    }
    interrupt->SetLevel(oldLevel);
 }
