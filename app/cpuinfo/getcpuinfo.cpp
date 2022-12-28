#include "getcpuinfo.h"


std::string GetCpuInfo() {
	std::array<int, 4> integerBuffer{};
	constexpr size_t sizeofIntegerBuffer = sizeof(int) * integerBuffer.size();

	std::array<char, 64> charBuffer = {};

	constexpr std::array<int, 3> functionIds = {
		0x8000'0002,
		0x8000'0003,
		0x8000'0004
	};

	std::string cpu;

	for (int id : functionIds) {
		__cpuid(integerBuffer.data(), id);
		std::memcpy(charBuffer.data(), integerBuffer.data(), sizeofIntegerBuffer);
		cpu += std::string(charBuffer.data());
	}

	return cpu;
}