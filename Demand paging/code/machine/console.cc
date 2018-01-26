// console.cc 
//	Routines to simulate a serial port to a console device.
//	A console has input (a keyboard) and output (a display).
//	These are each simulated by operations on UNIX files.
//	The simulated device is asynchronous,
//	so we have to invoke the interrupt handler (after a simulated
//	delay), to signal that a byte has arrived and/or that a written
//	byte has departed.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "console.h"
#include "system.h"

// Dummy functions because C++ is weird about pointers to member functions
static void ConsoleReadPoll(void* c) 
{ Console *console = (Console *)c; console->CheckCharAvail(); }
static void ConsoleWriteDone(void* c)
{ Console *console = (Console *)c; console->WriteDone(); }

//----------------------------------------------------------------------
// Console::Console
// 	Initialize the simulation of a hardware console device.
//
//	"readFile" -- UNIX file simulating the keyboard (NULL -> use stdin)
//	"writeFile" -- UNIX file simulating the display (NULL -> use stdout)
// 	"readAvail" is the interrupt handler called when a character arrives
//		from the keyboard
// 	"writeDone" is the interrupt handler called when a character has
//		been output, so that it is ok to request the next char be
//		output
//----------------------------------------------------------------------

Console::Console(const char *readFile, const char *writeFile,
		VoidFunctionPtr readAvail, 
		VoidFunctionPtr writeDone, void* callArg)
{
	printf("here\n");
    if (readFile == NULL)
	readFileNo = 0;					// keyboard = stdin
    else
    	readFileNo = OpenForReadWrite(readFile, true);	// should be read-only
    printf("here\n");
    if (writeFile == NULL)
	writeFileNo = 1;				// display = stdout
    else
    	writeFileNo = OpenForWrite(writeFile);
    printf("here\n");
    // set up the stuff to emulate asynchronous interrupts
    writeHandler = writeDone;
    readHandler = readAvail;
    handlerArg = callArg;
    printf("here\n");
    putBusy = false;
    incoming = EOF;
    printf("here\n");
    // start polling for incoming packets
    if( interrupt==NULL)
    {
        printf("Problem is here\n");
    }
    interrupt->Schedule(ConsoleReadPoll, this, ConsoleTime, ConsoleReadInt);
    printf("here\n");
}

//----------------------------------------------------------------------
// Console::~Console
// 	Clean up console emulation
//----------------------------------------------------------------------

Console::~Console()
{
    if (readFileNo != 0)
	Close(readFileNo);
    if (writeFileNo != 1)
	Close(writeFileNo);
}

//----------------------------------------------------------------------
// Console::CheckCharAvail()
// 	Periodically called to check if a character is available for
//	input from the simulated keyboard (eg, has it been typed?).
//
//	Only read it in if there is buffer space for it (if the previous
//	character has been grabbed out of the buffer by the Nachos kernel).
//	Invoke the "read" interrupt handler, once the character has been 
//	put into the buffer. 
//----------------------------------------------------------------------

void
Console::CheckCharAvail()
{
    char c;

    // schedule the next time to poll for a packet
    interrupt->Schedule(ConsoleReadPoll, this, ConsoleTime, 
			ConsoleReadInt);

    // do nothing if character is already buffered, or none to be read
    if ((incoming != EOF) || !PollFile(readFileNo))
	return;	  

    // otherwise, read character and tell user about it
    Read(readFileNo, &c, sizeof(char));
    incoming = c ;
    stats->numConsoleCharsRead++;
    (*readHandler)(handlerArg);	
}

//----------------------------------------------------------------------
// Console::WriteDone()
// 	Internal routine called when it is time to invoke the interrupt
//	handler to tell the Nachos kernel that the output character has
//	completed.
//----------------------------------------------------------------------

void
Console::WriteDone()
{
    putBusy = false;
    stats->numConsoleCharsWritten++;
    (*writeHandler)(handlerArg);
}

//----------------------------------------------------------------------
// Console::GetChar()
// 	Read a character from the input buffer, if there is any there.
//	Either return the character, or EOF if none buffered.
//----------------------------------------------------------------------

char
Console::GetChar()
{
   char ch = incoming;

   incoming = EOF;
   return ch;
}

//----------------------------------------------------------------------
// Console::PutChar()
// 	Write a character to the simulated display, schedule an interrupt 
//	to occur in the future, and return.
//----------------------------------------------------------------------

void
Console::PutChar(char ch)
{
    ASSERT(putBusy == false);
    WriteFile(writeFileNo, &ch, sizeof(char));
    putBusy = true;
    interrupt->Schedule(ConsoleWriteDone, this, ConsoleTime,
					ConsoleWriteInt);
}
