// display.h
// Aswin van Woudenberg

#ifndef DISPLAY_H
#define DISPLAY_H

#include "memory.h"

#include <cstddef>

class Display
{
private:
	bool (*display)[64];
	int width;
	int height;

	// Callback
	void (*update)(Display *);
public:
	Display();
	~Display();
	bool getPixel(int x, int y);
	void clear();
	void setUpdateProc(void (*p)(Display *));
	int getWidth();
	int getHeight();
	void setWidth(int w);
	void setHeight(int h);
	void setDimensions(int w, int h);
	void scrollDown(int lines);
	void scrollLeft();
	void scrollRight();
	bool drawSprite(byte x, byte y, Memory &memory, word offset, byte length);
	bool drawSprite(byte x, byte y, Memory &memory, word offset);
};

#endif
