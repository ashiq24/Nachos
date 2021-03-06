// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"

#include "memoryManager.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------
static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    //NoffHeader noffH;
    unsigned int i, size;
    this->executable=executable;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    this->actualsize=size;
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = -1;
	pageTable[i].valid = false;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on
					// a separate page, we could set its
					// pages to be read-only
    }

// zero out the entire address space, to zero the unitialized data segment
// and the stack segment
    //bzero(machine->mainMemory, size);


// then, copy in the code and data segments into memory
    /*if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }*/

    printf("code %d -- %d --- %d\n",noffH.code.virtualAddr ,noffH.code.size,noffH.code.inFileAddr);
    printf("data %d -- %d ---- %d \n",noffH.initData.virtualAddr ,noffH.initData.size,noffH.initData.inFileAddr);
    DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",noffH.code.virtualAddr, noffH.code.size,noffH.initData.inFileAddr);
    /*for( int i=0;i<noffH.code.size;i++)
    {
        executable->ReadAt(&(machine->mainMemory[virtophy(noffH.code.virtualAddr+i)]),1, noffH.code.inFileAddr+i);
    }
    DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",noffH.initData.virtualAddr, noffH.initData.size);
    for( int i=0;i<noffH.initData.size;i++)
    {
        executable->ReadAt(&(machine->mainMemory[virtophy(noffH.initData.virtualAddr+i)]),1, noffH.initData.inFileAddr+i);
    }
    /*int nInitPages = divRoundUp((noffH.code.size + noffH.initData.size), PageSize);												//& init data section into

	if (noffH.code.size > 0 || noffH.initData.size > 0)
    {
        printf("%d -- %d \n",noffH.code.virtualAddr ,noffH.code.size);
        printf("%d -- %d \n",noffH.initData.virtualAddr ,noffH.initData.size);
        int baseAddr = noffH.code.inFileAddr;

        for (int i = 0; i < nInitPages; i++)
        {
            executable -> ReadAt(&(machine -> mainMemory[pageTable[i].physicalPage * PageSize]),
                            PageSize, baseAddr + i * PageSize);
        }

    }*/

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
   delete executable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace :: loadIntoFreePage( int vadd)
{
    int vpn = (vadd/PageSize);
     printf("\n\n--------------loading page %d   %d------------\n\n",vpn,vadd);
     int r =memorymanager->load(vpn,this->uniqid);
     pageTable[vpn].dirty = false;
     pageTable[vpn].valid=true;
     pageTable[vpn].physicalPage = r;

     return;
}
void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}
//memory translation
int AddrSpace :: virtophy( int vir)
{

	int pagelok = pageTable[vir/PageSize].physicalPage * PageSize ;
	return pagelok+ (vir %PageSize);
}
//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------
void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

void AddrSpace :: loadfromFile( int vpn, int phys)
{
	int nInitPages = divRoundUp((noffH.code.size + noffH.initData.size), PageSize);
	int baseAddr = noffH.code.inFileAddr;
    //printf("loading from %d to %d\n",40+vpn*PageSize,40+vpn*PageSize+PageSize );
    if(nInitPages>vpn) {
        int k=executable -> ReadAt(&(machine -> mainMemory[phys* PageSize]),
                            PageSize, baseAddr + vpn * PageSize);
        //printf("---> read %d\n", k);
    }
    else {
        //printf("outside initdata\n");
    }
}

void  AddrSpace::FreeMemory()
{
    for(int i=0;i< numPages ; i++)
    {
        if(pageTable[i].valid==true)memorymanager->FreePage(pageTable[i].physicalPage);
    }

}
