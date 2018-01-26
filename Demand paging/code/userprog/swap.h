#ifndef SWAP_H
#define SWAP_H
#include "system.h"
#include "ProcessTable.h"
#include "copyright.h"
#include "filesys.h"
#include <string>
#include <sstream>
class Swap{
public:
    OpenFile** swapFile;
    int ** entry;
    int * sizes;
    Swap(int numpros)
    {
        numpros++;
        swapFile = new OpenFile*[numpros];
        entry = new int*[numpros];
        sizes = new int[numpros];

        for( int i=0;i<numpros;i++)
        {
            swapFile[i]=NULL;
            entry[i]=NULL;
            sizes[i]= -1;
        }
    }
    ~Swap()
    {
        delete swapFile;
        delete entry;
    }

    void StoreIntoSwap(int vpn,int prosid,int pysical)
    {
        if(swapFile[prosid]==NULL)
        {
            printf("OPENING NEW SWAP FILE \n");
            Thread * thread = (Thread *)processtable->Get(prosid);
            int tsize= thread->space->numPages;
            entry[prosid]= new int[tsize];
            for( int n=0; n<tsize;n++) entry[prosid][n]=0;
            sizes[prosid]=tsize;
            std::stringstream ss;
            ss << prosid;
            std :: string s = ss.str();
            fileSystem->Create(s.c_str(), tsize*PageSize+8);
            swapFile[prosid] = fileSystem->Open(s.c_str());
            printf("DONE OPENING \n");


        }
        swapFile[prosid]->WriteAt(machine->mainMemory + pysical * PageSize, PageSize, vpn * PageSize);
        entry[prosid][vpn]=1;
    }

    bool ispresent(int vpn, int prosid)
    {
        if(swapFile[prosid]==NULL) return false;
        //printf("swap status\n");
        //for( int n=0;n<sizes[prosid];n++) printf("%d ->",entry[prosid][n] );
        //printf("\n");
        if( entry[prosid][vpn]==0){

            return false;
        } 
        else return true;
    }

    void loadtomemory(int vpn, int prosid, int physical)
    {
        bzero( machine->mainMemory + physical * PageSize, PageSize );
        swapFile[prosid]->ReadAt(machine->mainMemory + physical * PageSize, PageSize, vpn * PageSize);
    }

    void removeswap(int prosid)
    {
        if(swapFile[prosid]==NULL) return;
        delete swapFile[prosid];
        delete entry[prosid];
        swapFile[prosid]=NULL;
        std::stringstream ss;
        ss << prosid;
        std :: string s = ss.str();
        fileSystem->Remove(s.c_str());

    }
};

extern Swap* swapspace;
#endif // SWAP_H
