#pragma once

#include <chrono>

struct Timer
{
	Timer();
	virtual ~Timer() = default;

	float Get();
	float GetAndReset();
protected:
	std::chrono::steady_clock::time_point Start;
};