// memory.h
// Aswin van Woudenberg

#ifndef MEMORY_H
#define MEMORY_H

#define FONT_OFFSET 0x20
#define BIG_FONT_OFFSET 0x100
#define PROGRAM_OFFSET 0x200

typedef unsigned char byte;
typedef unsigned short word;

typedef union {
   struct {
      unsigned char nibble0 : 4;
      unsigned char nibble1 : 4;
      unsigned char nibble2 : 4;
      unsigned char nibble3 : 4;
   };
   struct {
      unsigned short address : 12;
      unsigned short opcode : 4;
   };
   struct {
      byte lo_byte;
      byte hi_byte;
   };
   word value;
} Instruction;

class Memory
{
private:
	byte *memory;
public:
	Memory();
	~Memory();
	bool loadProgram(const char fileName[]);
	bool loadProgram(const char fileName[], word offset);
	void loadFont();
	void loadFontChip8();
	void loadFontSChip48();
	word getCharacterAddress(byte num);
	word getBigCharacterAddress(byte num);
	word getWord(word address);
	void setWord(word address, word value);
	byte getByte(word address);
	void setByte(word address, byte value);
	Instruction getInstruction(word address);
};

#endif
