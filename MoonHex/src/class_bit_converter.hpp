#pragma once

#include <string>

class BitConverter
{
private:
	BitConverter() = default;
	~BitConverter() = default;
public:
	static char* ToHexString(uint32_t value);
	static char* ToHexString(uint8_t value);
private:
	static char* s_Nibbles;
};