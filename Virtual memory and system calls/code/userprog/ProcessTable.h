#ifndef PROCESSTABLE
#define PROCESSTABLE
#include "synch.h"
#include "syscall.h"
class ProcessTable{

    public:
    void ** processes;
    int numpros;
    int maxpros;
    Lock * lock;
    /* Create a table to hold at most "size" entries. */
    ProcessTable(int size)
    {
        size++;
        processes = new void*[size];
        maxpros=size;
        for( int n=0;n<size;n++)
        {
            processes[n]=NULL;
        }
        lock = new Lock("table lock");
    }

    /* Allocate a table slot for "object", returning the "index" of the
       allocated entry; otherwise, return -1 if no free slots are available. */
    int Alloc(void *object)
    {
        lock->Acquire();
        if( numpros==maxpros)
        {
            lock->Release();
            printf(" not more processes allowed\n");
            return -1;
        }
        else{
            numpros++;
            for( int n = 0 ; n< maxpros ;n++)
            {
                 if( processes[n]==NULL)
                 {
                    processes[n]=object;
                    printf("number of processes %d \n", numpros);
                    lock->Release();
                    return n;

                 }
            }
        }
    }


    /* Retrieve the object from table slot at "index", or NULL if that
       slot has not been allocated. */
    void *Get(int index)
    {
        return processes[index];
    }


    void Release(int index){
        lock->Acquire();
        processes[index]= NULL;
        numpros--;
        lock->Release();
        printf("number of processes remain %d \n", numpros);

    }

};
extern ProcessTable * processtable;
extern Semaphore *readAvail;
extern Semaphore *writeDone ;
extern void ReadAvail(void* arg) ;
extern void WriteDone(void* arg) ;
extern Console *console;
#endif // PROCESSTABLE


