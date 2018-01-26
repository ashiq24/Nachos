#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "bitmap.h"
#include "synch.h"

class MemoryManager
{
    public:
    BitMap * bitmap;
    Lock * lock;
    MemoryManager(int numpages){
    bitmap =new BitMap(numpages);
    lock = new Lock("lockmemory");
    }
    /* Allocate a free page, returning its physical page number or -1
    if there are no free pages available. */
    int   AllocPage(){
    lock->Acquire();
    int pagenumber = bitmap->Find();
    lock->Release();
    return pagenumber;

    }

    /* Free the physical page and make it available for future allocation. */
void  FreePage(int physPageNum)
{
    lock->Acquire();
    bitmap->Clear(physPageNum);
    lock->Release();

}

bool PageIsAllocated(int physPageNum)
{
    lock->Acquire();
    bool pagestate = bitmap->Test(physPageNum);
    lock->Release();
    return pagestate;

}

int  Freememorysize()
{
    lock->Acquire();
    int free = bitmap->NumClear();
    lock->Release();
    return free;
}
};

extern MemoryManager* memorymanager;
#endif
