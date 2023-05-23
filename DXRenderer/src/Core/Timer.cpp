#include "Timer.h"

Timer::Timer()
{
	Start = std::chrono::steady_clock::now();
}

float Timer::Get()
{
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - Start).count();
}

float Timer::GetAndReset()
{
	auto temp = Start;
	Start = std::chrono::steady_clock::now();
	return  std::chrono::duration<float>(std::chrono::steady_clock::now() - temp).count();
}
