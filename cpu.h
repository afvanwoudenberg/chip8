// cpu.h
// Aswin van Woudenberg

#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include "display.h"
#include "keyboard.h"

class Cpu
{
private:
	// IO
	Memory *memory;
	Display *display;
	Keyboard *keyboard;

	// Registers
	byte v[16];
	word i;

	word programCounter;
	word stackPointer;
	
	byte st;
	byte dt;

	// Callback
	void (*startBeep)();
	void (*stopBeep)();
	void (*processKeyboard)();
public:
	Cpu();
	void setDisplay(Display *d);
	void setMemory(Memory *m);
	void setKeyboard(Keyboard *k);
	void setStartBeepProc(void (*p)());
	void setStopBeepProc(void (*p)());
	void setKeyboardProc(void (*p)());
	void reset();
	bool interpret(int numberOfInstructions);
};

#endif
