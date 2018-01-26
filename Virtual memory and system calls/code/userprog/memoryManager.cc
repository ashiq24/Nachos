#include "memoryManager.h"
extern  MemoryManager * memorymanager;
MemoryManager :: MemoryManager(int numpages){
    bitmap =new BitMap(numpages);
    lock = new Lock("lockmemory");
}

int MemoryManager ::  AllocPage(){
    lock->Acquire();
    int pagenumber = bitmap->Find();
    lock->Release();
    return pagenumber;

}

void MemoryManager :: FreePage(int physPageNum)
{
    lock->Acquire();
    bitmap->Clear(physPageNum);
    lock->Release();

}

bool MemoryManager :: PageIsAllocated(int physPageNum)
{
    lock->Acquire();
    bool pagestate = bitmap->Test(physPageNum);
    lock->Release();
    return pagestate;

}

int MemoryManager :: Freememorysize()
{
    lock->Acquire();
    int free = bitmap->NumClear();
    lock->Release();
    return free;
}






