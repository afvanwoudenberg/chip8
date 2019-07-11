// cpu.cpp
// Aswin van Woudenberg

#include "cpu.h"

#include <cstdlib>
#include <ctime>

Cpu::Cpu()
{
	startBeep = NULL;
	stopBeep = NULL;
	processKeyboard = NULL;
	display = NULL;
	memory = NULL;
	keyboard = NULL;
	reset();
}

void Cpu::setDisplay(Display *d) 
{
	display = d;
}

void Cpu::setMemory(Memory *m) 
{
	memory = m;
}

void Cpu::setKeyboard(Keyboard *k)
{
	keyboard = k;
}

void Cpu::setStartBeepProc(void (*p)())
{
	startBeep = p;
}

void Cpu::setStopBeepProc(void (*p)())
{
	stopBeep = p;
}

void Cpu::setKeyboardProc(void (*p)())
{
	processKeyboard = p;
}

void Cpu::reset() 
{
	srand((unsigned)time(NULL));
	programCounter = 0x200;
	stackPointer = 0xF;
	st = 0;
	dt = 0;
	for (int r=0; r<16; r++)
		v[r] = 0;
	if (display != NULL) {
		display->setDimensions(64, 32);
		display->clear();
	}
}

bool Cpu::interpret(int numberOfInstructions)
{
	for (int c=0; c<numberOfInstructions; c++)
	{
		Instruction instruction = memory->getInstruction(programCounter);
		programCounter += 2;
		if (dt > 0) dt--;
		if (st > 0) {
			st--;
			if (st == 0 && stopBeep != NULL)
				stopBeep();
		}
		switch (instruction.opcode) 
		{
		case 0x0: 
			{
				switch (instruction.lo_byte)
				{
				case 0xC0: case 0xC1: case 0xC2: case 0xC3: 
				case 0xC4: case 0xC5: case 0xC6: case 0xC7: 
				case 0xC8: case 0xC9: case 0xCA: case 0xCB: 
				case 0xCC: case 0xCD: case 0xCE: case 0xCF: // SCD nibble
					{
						display->scrollDown(instruction.nibble0);
						break;
					}
				case 0xE0: // CLS
					{
						display->clear();
						break;
					}
				case 0xEE: // RET
					{
						programCounter = memory->getWord(stackPointer);
						stackPointer -= 2;
						break;
					}
				case 0xFB: // SCR
					{
						display->scrollRight();
						break;
					}
				case 0xFC: // SCL
					{
						display->scrollLeft();
						break;
					}
				case 0xFD: // EXIT
					{
						programCounter -= 2;
						return false;
					}
				case 0xFE: // LOW
					{
						display->setDimensions(64, 32);
						break;
					}
				case 0xFF: // HIGH
					{
						display->setDimensions(128, 64);
						break;
					}
				default:
					{
						// Unimplemented instruction
						return false;
					}
				}
				break;
			}
		case 0x1: // JP addr
			{
				programCounter = instruction.address;
				break;
			}
		case 0x2: // CALL addr
			{
				stackPointer += 2;
				memory->setWord(stackPointer, programCounter);
				programCounter = instruction.address;
				break;
			}
		case 0x3: // SE Vx, byte
			{
				if (v[instruction.nibble2] == instruction.lo_byte)
					programCounter += 2;
				break;
			}
		case 0x4: // SNE Vx, byte
			{
				if (v[instruction.nibble2] != instruction.lo_byte)
					programCounter += 2;
				break;
			}
		case 0x5: // SE Vx, Vy
			{
				if (v[instruction.nibble2] == v[instruction.nibble1])
					programCounter += 2;
				break;
			}
		case 0x6: // LD VX, byte
			{
				v[instruction.nibble2] = instruction.lo_byte;
				break;
			}
		case 0x7: // ADD Vx, byte
			{
				v[instruction.nibble2] += instruction.lo_byte;
				break;
			}
		case 0x8:
			{
				switch (instruction.nibble0)
				{
				case 0x0: // LD Vx, Vy
					{
						v[instruction.nibble2] = v[instruction.nibble1];
						break;
					}
				case 0x1: // OR Vx, Vy
					{
						v[instruction.nibble2] |= v[instruction.nibble1];
						break;
					}
				case 0x2: // AND Vx, Vy
					{
						v[instruction.nibble2] &= v[instruction.nibble1];
						break;
					}
				case 0x3: // XOR Vx, Vy
					{
						v[instruction.nibble2] ^= v[instruction.nibble1];
						break;
					}
				case 0x4: // ADD Vx, Vy
					{
						int result = (int)v[instruction.nibble2] + (int)v[instruction.nibble1];
						v[instruction.nibble2] += v[instruction.nibble1];
						if (result > 255) {
							v[0xF] = 1;
						} else {
							v[0xF] = 0;
						}
						break;
					}
				case 0x5: // SUB Vx, Vy
					{
						if (v[instruction.nibble2] > v[instruction.nibble1])
							v[0xF] = 1;
						else
							v[0xF] = 0;
						v[instruction.nibble2] -= v[instruction.nibble1];
						break;
					}
				case 0x6: // SHR Vx
					{
						v[0xF] = (v[instruction.nibble2] & 0x1);
						v[instruction.nibble2] >>= 1;
						break;
					}
				case 0x7: // SUBN Vx, Vy
					{
						if (v[instruction.nibble1] > v[instruction.nibble2]) {
							v[0xF] = 1;
						} else {
							v[0xF] = 0;
						}
						v[instruction.nibble2] = v[instruction.nibble1] - v[instruction.nibble2];
						break;
					}
				case 0xE: // SHL Vx
					{
						v[0xF] = ((v[instruction.nibble2] >> 7) & 0x1);
						v[instruction.nibble2] <<= 1;
						break;
					}
				default:
					{
						// Unimplemented instruction
						return false;
					}
				}
				break;
			}
		case 0x9:
			{
				switch (instruction.nibble0)
				{
				case 0x0: // SNE Vx, Vy
					{
						if (v[instruction.nibble2] != v[instruction.nibble1])
							programCounter += 2;
						break;
					}
				default:
					{
						// Unimplemented instruction
						return false;
					}
				}
				break;
			}
		case 0xA: // LD I, addr
			{
				i = instruction.address;
				break;
			}
		case 0xB: // JP V0, addr
			{
				programCounter = instruction.address + v[0];
				break;
			}
		case 0xC: // RND Vx, byte
			{
				v[instruction.nibble2] = (rand()%256) & instruction.lo_byte;
				break;
			}
		case 0xD: // DRW Vx, Vy, nibble
			{
				if (instruction.nibble0 > 0) {
					if (display->drawSprite(v[instruction.nibble2], v[instruction.nibble1], *memory, i, instruction.nibble0))
						v[0xF] = 1;
					else
						v[0xF] = 0;
				} else { // Draw a sprite that's 16x16 pixels
					if (display->drawSprite(v[instruction.nibble2], v[instruction.nibble1], *memory, i))
						v[0xF] = 1;
					else
						v[0xF] = 0;
				}
				break;
			}
		case 0xE:
			{
				switch (instruction.lo_byte)
				{
				case 0x9E: // SKP Vx
					{
						if (processKeyboard != NULL)
							processKeyboard();
						if (keyboard->keyPressed(v[instruction.nibble2]))
							programCounter += 2;
						break;
					}
				case 0xA1: // SKNP Vx
					{
						if (processKeyboard != NULL)
							processKeyboard();
						if (!keyboard->keyPressed(v[instruction.nibble2]))
							programCounter += 2;
						break;
					}
				default:
					{
						// Unimplemented instruction
						return false;
					}
				}
				break;
			}
		case 0xF:
			{
				switch(instruction.lo_byte)
				{
				case 0x7: // LD Vx, DT
					{
						v[instruction.nibble2] = dt;
						break;
					}
				case 0xA: // LD Vx, K
					{
						bool keyPressed = false;
						for (byte k=0; k<16; k++)
							if (keyboard->keyPressed(k)) {
								keyPressed = true;
								keyboard->releaseAllKeys();
								v[instruction.nibble2] = k;
							}
						if (!keyPressed)
							programCounter -= 2;
						break;
					}
				case 0x15: // LD DT, Vx
					{	
						dt = v[instruction.nibble2];
						break;
					}
				case 0x18: // LD ST, Vx
					{
						st = v[instruction.nibble2];
						if (st > 0 && startBeep != NULL)
							startBeep();
						break;
					}
				case 0x1E: // ADD I, Vx
					{
						i += v[instruction.nibble2];
						break;
					}
				case 0x29: // LD F, Vx
					{
						i = memory->getCharacterAddress(v[instruction.nibble2]);
						break;
					}
				case 0x30: // LD HF, Vx
					{
						i = memory->getBigCharacterAddress(v[instruction.nibble2]);
						break;
					}
				case 0x33: // LD B, Vx
					{
						byte num = v[instruction.nibble2];
						memory->setByte(i+2, num % 10);
						num /= 10;
						memory->setByte(i+1, num % 10);
						num /= 10;
						memory->setByte(i, num % 10);
						break;
					}
				case 0x55: // LD [I], Vx
					{
						for (byte b=0; b<=instruction.nibble2; b++)
							memory->setByte(b + i, v[b]);
						break;
					}
				case 0x65: // LD Vx, [I]
					{
						for (byte b=0; b<=instruction.nibble2; b++)
							v[b] = memory->getByte(b + i);
						break;
					}
				default:
					{
						// Unimplemented instruction
						return false;
					}
				}
				break;
			}
		default:
			{
				// Unimplemented instruction
				return false;
			}
		}
	}
	return true;
}
