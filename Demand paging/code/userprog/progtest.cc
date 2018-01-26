// progtest.cc
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "memoryManager.h"
#include "ProcessTable.h"
#include "console.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
static Semaphore *readAvail=new Semaphore("read avail", 0);
static Semaphore *writeDone =  new Semaphore("write done", 0);
static void ReadAvail(void* arg) { readAvail->V(); }
static void WriteDone(void* arg) { writeDone->V(); }
static Console *console;
MemoryManager * memorymanager = new MemoryManager(NumPhysPages);
ProcessTable * processtable = new ProcessTable( 10 );
Swap* swapspace = new Swap(10);

 Semaphore *rreadAvail;
 Semaphore *wwriteDone ;
 void RReadAvail(void* arg){ rreadAvail->V(); }
 void WWriteDone(void* arg) { wwriteDone->V(); }
 Console *cconsole;

void
StartProcess(const char *filename)
{
    rreadAvail =new Semaphore("read avail", 0);
    wwriteDone =  new Semaphore("write done", 0);
    cconsole = new Console(NULL,NULL,RReadAvail,WWriteDone,0);
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace(executable);
    currentThread->space = space;
    int id = processtable->Alloc(currentThread);
    currentThread->space->uniqid=id;
    printf("\n--------------%d-----------\n\n",id);

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(false);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.



//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void
ConsoleTest (const char *in, const char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
