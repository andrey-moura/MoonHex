#include "class_bit_converter.hpp"

char* BitConverter::s_Nibbles = new char[16]{ '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

char* BitConverter::ToHexString(uint32_t value)
{
	char* offsetText = new char[9];	

	uint32_t mask = 0xF0000000;
	uint8_t index = 28;

	for (size_t i = 0; i < 8; ++i)
	{		
		offsetText[i] = s_Nibbles[(value & mask) >> index];
		mask = mask >> 4;
		index -= 4;
	}
	offsetText[8] = '\0';

	return offsetText;
}

char* BitConverter::ToHexString(uint8_t value)
{
	char* byteText = new char[3];
	byteText[0] = s_Nibbles[(value & 0xF0) >> 4];
	byteText[1] = s_Nibbles[value & 0x0F];
	byteText[2] = 0x0;

	return byteText;
}