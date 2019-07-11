// keyboard.cpp
// Aswin van Woudenberg

#include "keyboard.h"

Keyboard::Keyboard()
{
	releaseAllKeys();
}

void Keyboard::pressKey(byte num)
{
	releaseAllKeys();
	key[num] = true;
}

void Keyboard::releaseKey(byte num)
{
	if (key[num])
		releaseAllKeys();
}

void Keyboard::releaseAllKeys() 
{
	for (byte b=0; b<16; b++)
		key[b] = false;
}

bool Keyboard::keyPressed(byte num)
{
	return key[num];
}
