// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "ProcessTable.h"
#include "console.h"
#include "synch.h"
#include "swap.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------
Lock * mylock = new Lock("console");
int numberPageFault=0;

void INCPC()
{
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
}

void Initialization(void * a)
{
    currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
	return;

}
void
SysCallExitHandler()
{
	int arg = machine->ReadRegister(4);
	currentThread->space->FreeMemory();
    printf("Process exit with code %d \n", arg );
    processtable->Release(currentThread->space->uniqid);
    swapspace->removeswap(currentThread->space->uniqid);
    if(processtable->numpros==0)
    {
    	printf("NO RUNNING PROCESS\n");
        printf("Total number of page Fault %d \n", numberPageFault );
    	interrupt->Halt();
    }
	currentThread->Finish();

	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);

	return;
}
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    }
    else if((which == SyscallException))
    {
		//printf("user mode exception %d %d\n", which, type);
		//ASSERT(false);
		/*printf("Just halting \n");
		interrupt->Halt();*/
		switch(type)
		{
            case SC_Exit :
                SysCallExitHandler();
                break;

            case SC_Exec:
                {
                char fileName[100];
                int i=0;
                int pos = machine->ReadRegister(4);
                int * val = new int;
                if(!machine->ReadMem(pos, 1, val)){
                	machine->ReadMem(pos, 1, val);
                }
                //printf("here 1\n");
                while ((*(char*)val) != '\0') {
                    fileName[i]  = (char)(*val);
                    i++;
                    pos++;
                    if(!machine->ReadMem(pos, 1, val)){
                		machine->ReadMem(pos, 1, val);
                	}
                }
                //printf("here 1\n");
                fileName[i]  = (char)(*val);
                OpenFile *executable = fileSystem->Open(fileName);
                AddrSpace *space;

                if (executable == NULL) {
                    printf("Unable to open file ---%s-----\n", fileName);
                    INCPC();
                    return;
                }
                Thread * execThraed = new Thread( fileName);
                space = new AddrSpace(executable);
                execThraed->space=space;
                int status = processtable->Alloc(execThraed);
                if ( status == -1)
                {
                    printf("unable to create new process\n");
                    machine->WriteRegister(2, status);
                    INCPC();
                    return ;
                }
                execThraed->space->uniqid=status;
                machine->WriteRegister(2, status);
                printf("NEW PROCESS IS FORKED %s\n",fileName );
                execThraed->Fork(Initialization, NULL);
                INCPC();
                return;



                }
                break;
            case SC_Write:
                {
                //printf("------------inside print---------------\n");
                int mem = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                int fileid= machine->ReadRegister(6);
                if(fileid!=ConsoleOutput) ASSERT(false);
                mylock->Acquire();
                int * temp = new int;
                for(int i = 0; i < size; i++)
                {
                	char * c = new char;
                	if(!machine->ReadMem(mem+i,1,(int*)c)){
                		machine->ReadMem(mem+i,1,(int*)c);

                	}
                	
                    cconsole->PutChar(*c);
                    wwriteDone->P();
	                

                }
                printf("\n\n");
                mylock->Release();
               // printf("--------------write done---------------------\n");
                INCPC();
                }
                break;
            case SC_Read:
            {
                //printf("-------------------inside read--------------------\n");
                int mem = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                int fileid= machine->ReadRegister(6);
                if(fileid!=ConsoleInput) ASSERT(false);
                int actread=0;
                
                mylock->Acquire();
                for(int i = 0; i<size; i++)
                {
                    rreadAvail->P();
                    char * c = new char;
                    *c=cconsole->GetChar();
                    
                    if(!machine->WriteMem(mem+i,1, (int)(*c))){
                    	machine->WriteMem(mem+i,1, (int)(*c));
                    }
                    
                    actread++;
                    if(*c==EOF || *c=='\n') {
                        break;
                        //printf("file ends\n");
                    }
                	
                    
        
                }
                machine->WriteRegister(2,actread);
                mylock->Release();
                //printf("-------------------READ DONE--------------------\n");
                
                INCPC();
                

            }
                break;
            default:
                printf("no way out\n");
                INCPC();


		}

    }
    else if( which == PageFaultException)
    {

        int addr =  machine->ReadRegister(39);
        numberPageFault++;
        currentThread->space->loadIntoFreePage(addr);

    }
}
