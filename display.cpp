// display.cpp
// Aswin van Woudenberg

#include "display.h"

Display::Display()
{
	display = new bool[128][64];
	setDimensions(64, 32);
	update = NULL;
	clear();
}

Display::~Display()
{
	delete [] display;
}

bool Display::getPixel(int x, int y)
{
	return display[x][y];
}

void Display::clear()
{
	for (int x=0; x<128; x++)
		for (int y=0; y<64; y++)
			display[x][y] = false;
	if (update != NULL) 
		update(this);
}

void Display::setUpdateProc(void (*p)(Display *))
{
	update = p;
}

int Display::getWidth()
{
	return width;
}

int Display::getHeight()
{
	return height;
}

void Display::setWidth(int w)
{
	width = w;
}

void Display::setHeight(int h)
{
	height = h;
}

void Display::setDimensions(int w, int h)
{
	width = w;
	height = h;
}

void Display::scrollDown(int lines)
{
	for (int y=height-1; y>=lines; y--)
		for (int x=0; x<width; x++)
			display[x][y] = display[x][y-lines];
	for (int y=0; y<lines; y++)
		for (int x=0; x<width; x++)
			display[x][y] = false;
	if (update != NULL) 
		update(this);
}

void Display::scrollLeft()
{
	int delta = (width == 64 ? 2 : 4);
	for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
			if (x<width-delta) {
				display[x][y] = display[x+delta][y];
			} else {
				display[x][y] = false;
			}
	if (update != NULL) 
		update(this);
}

void Display::scrollRight()
{
	int delta = (width == 64 ? 2 : 4);
	for (int x=width-1; x>=0; x--)
		for (int y=0; y<height; y++)
			if (x>delta) {
				display[x][y] = display[x-delta][y];
			} else {
				display[x][y] = false;
			}
	if (update != NULL) 
		update(this);
}

bool Display::drawSprite(byte x, byte y, Memory &memory, word offset, byte length)
{
	byte vf = 0;
	static int deleteCount = 0;
	static int passes = 0;
	bool deleteOnly = true;
	for (int i=0; i<length; i++)
	{
		byte b = memory.getByte(offset + i);
		for (int j=0; j<8; j++)
		{
			if ((display[(x + j) % width][(y + i) % height]) && (((b >> (7 - j)) & 1) == 1))
				vf = 1;
			if (!(display[(x + j) % width][(y + i) % height]) && (((b >> (7 - j)) & 1) == 1))
				deleteOnly = false;
			display[(x + j) % width][(y + i) % height] ^= (((b >> (7 - j)) & 1) == 1);
		}
	}
	if (deleteOnly) 
	{ 
		deleteCount++;
		passes++;
	}
	else 
	{	if (deleteCount > 0)
			deleteCount--;
	}
	if (passes > 2) {
		deleteCount = 0;
		passes = 0;
	}
	if (update != NULL && (deleteCount == 0)) 
		update(this);
	return (vf == 1);
}

bool Display::drawSprite(byte x, byte y, Memory &memory, word offset)
{	// Draw a sprite that's 16x16 pixels
	byte vf = 0;
	static int deleteCount = 0;
	static int passes = 0;
	bool deleteOnly = true;
	for (int i=0; i<16; i++)
	{
		word w = memory.getWord(offset + i*2);
		for (int j=0; j<16; j++)
		{
			if ((display[(x + j) % width][(y + i) % height]) && (((w >> (15 - j)) & 1) == 1))
				vf = 1;
			if (!(display[(x + j) % width][(y + i) % height]) && (((w >> (15 - j)) & 1) == 1))
				deleteOnly = false;
			display[(x + j) % width][(y + i) % height] ^= (((w >> (15 - j)) & 1) == 1);
		}
	}
	if (deleteOnly) 
	{ 
		deleteCount++;
		passes++;
	}
	else 
	{	if (deleteCount > 0)
			deleteCount--;
	}
	if (passes > 2) {
		deleteCount = 0;
		passes = 0;
	}
	if (update != NULL && (deleteCount == 0)) 
		update(this);
	return (vf == 1);
}
