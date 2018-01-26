#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "bitmap.h"
#include "synch.h"
#include "swap.h"
#include "system.h"
#include "addrspace.h"
#include <stdlib.h>
class MemoryManager
{
    public:
    int numpages;
    BitMap * bitmap;
    Lock * lock;
    int * frametopros;
    int * frametovpn;
    int * lastaccess;
    int temptime;
    MemoryManager(int numpages)
    {
        this->numpages=numpages;
        bitmap =new BitMap(numpages);
        lock = new Lock("lockmemory");
        frametopros = new int[numpages];
        frametovpn = new int[numpages];
        lastaccess = new int[numpages];
        temptime=0;
        for( int i=0;i< numpages;i++)
        {
         frametopros[i]=-1;
         frametovpn[i]=-1;
         lastaccess[i]=0;
        }

    }
    /* Allocate a free page, returning its physical page number or -1
    if there are no free pages available. */
    int   AllocPage()
    {
       
        int pagenumber = bitmap->Find();
        return pagenumber;

    }
    int replacement()
    {
        
        int r= rand();
        r= r%numpages;
        r= findforLRU();

        printf("SELECTED PHYSICAL PAGE  %d \n",r);
        int rpros=frametopros[r];
        int vpn = frametovpn[r];
        Thread * thread = (Thread * )processtable->Get(rpros);
        TranslationEntry *rpageTable = thread->space->pageTable;
        rpageTable[vpn].physicalPage = -1;
        rpageTable[vpn].valid = false;
        if(rpageTable[vpn].dirty | !swapspace->ispresent(vpn,rpros)) swapspace->StoreIntoSwap(vpn,rpros,r);
       
        return r;
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

    int load(int vpn, int prosid)
    {
        lock->Acquire();
        int ppage= this->AllocPage();
        if(ppage==-1)
        {
            printf("NEED TO REPACLE PAGE \n");
            ppage= this->replacement();

        }
        //printf("Swapped the page\n");
        bzero( (machine->mainMemory+(ppage* PageSize)), PageSize);
        bool pbit = swapspace->ispresent(vpn,prosid);
        if( pbit==true)
        {
            printf("LOADED FROM SWAPFILE\n");
            swapspace->loadtomemory(vpn,prosid,ppage);
        }
        else
        {
            printf("LOADED FROM FILE \n");
            loadfromfile(vpn, prosid,ppage);
        }
        frametopros[ppage]=prosid;
        frametovpn[ppage]=vpn;
        //lastaccess[ppage]=0;
        lock->Release();
        return ppage;

    }
    void IncTime( int phys)
    {
        lock->Acquire();
        this->temptime++;
        lastaccess[phys]=temptime;
        lock->Release();
    }
    int findforLRU()
    {
        int tlow=lastaccess[0], r=0;
        for( int n =0; n<numpages;n++)
        {
            printf("%d ",lastaccess[n] );
            if(tlow>lastaccess[n])
            {
                tlow=lastaccess[n];
                r=n;
            }

        }
        printf("\n");
        return r;
    }
    void loadfromfile(int vpn, int prosid, int phys)
    {
        Thread * thread = ( Thread*) processtable->Get(prosid);
        thread->space->loadfromFile( vpn,  phys);
    }
};

extern MemoryManager* memorymanager;
#endif
