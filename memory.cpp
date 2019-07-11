// memory.cpp
// Aswin van Woudenberg

#include <iostream>
#include <fstream>

#include "memory.h"
#include "cpu.h"

using namespace std;

Memory::Memory()
{
	memory = new byte[4 * 1024];
	loadFont();
}

Memory::~Memory()
{
	delete [] memory;
}

bool Memory::loadProgram(const char fileName[])
{
	return loadProgram(fileName, PROGRAM_OFFSET);
}

bool Memory::loadProgram(const char fileName[], word offset)
{
	ifstream::pos_type size;
	
	ifstream file(fileName, ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{
		size = file.tellg();
		if (size <= (4096 - offset)) 
		{
			// the file will fit in memory.
			memset(memory + offset, 0, 4096 - offset);
			file.seekg(0, ios::beg);
			file.read((char *)memory + offset, size);
			file.close();
			return true;
		}     
	}
	return false;
}

void Memory::loadFont()
{
	loadFontChip8();
	loadFontSChip48();
}

void Memory::loadFontChip8()
{
	byte font[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80 };
	for (int i = 0; i < 80; i++)
		memory[FONT_OFFSET + i] = font[i];
}

void Memory::loadFontSChip48()
{
	byte font[] = {
		0xF0, 0xF0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0, // 0
		0x20, 0x20, 0x60, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0x70,
		0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0,
		0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0,
		0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0, 0x10, 0x10, 0x10, 0x10,
		0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0,
		0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0,
		0xF0, 0xF0, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40,
		0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0,
		0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0 };
	for (int i = 0; i < 100; i++)
		memory[BIG_FONT_OFFSET + i] = font[i];
}

word Memory::getCharacterAddress(byte num)
{
	return FONT_OFFSET + (num * 5);
}

word Memory::getBigCharacterAddress(byte num)
{
	return BIG_FONT_OFFSET + (num * 10);
}

word Memory::getWord(word address)
{
	return (((word)memory[address & 0xFFF])<<8) + memory[(address & 0xFFF)+1];
}

void Memory::setWord(word address, word value)
{
	byte lo_byte = value & 0xFF;
	byte hi_byte = (value & 0xFF00) >> 8;
	memory[address] = hi_byte;
	memory[address + 1] = lo_byte;
}

byte Memory::getByte(word address)
{
	return memory[address & 0xFFF];
}

void Memory::setByte(word address, byte value)
{
	memory[address & 0xFFF] = value;
}

Instruction Memory::getInstruction(word address)
{
	Instruction instruction;
	instruction.value = getWord(address);
	return instruction;
}
