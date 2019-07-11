// keyboard.h
// Aswin van Woudenberg

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "memory.h"

class Keyboard
{
private:
	bool key[16];
public:
	Keyboard();
	void pressKey(byte num);
	void releaseKey(byte num);
	void releaseAllKeys();
	bool keyPressed(byte num);
};

#endif
