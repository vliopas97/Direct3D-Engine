#pragma once

#include "Buffer.h"
#include "Core/Core.h"
#include "Graphics.h"
#include "State.h"
#include "Utilities.h"

#include <vector>

class Step
{
public:
	Step(size_t pass);

	template<typename T, typename... Args>
	void Add(Args&&... args)
	{
		Resources.Add<T>(std::forward<Args>(args)...);
	}

	void Bind() const;
	void Submit(const GPUObject& renderObject) const;

private:
	size_t PassNumber;
	GPUObject Resources;
};

class Task
{
public:
	Task(const GPUObject* renderObject, const Step* step);
	void Execute() const;

private:
	const GPUObject* RenderObject;
	const Step* TStep;
};

class Technique
{
public:
	void Submit(const GPUObject& renderObject);

	inline void Activate() { IsActive = true; }
	inline void Deactivate() { IsActive = false; }

	void PushBack(Step&& step);

private:
	bool IsActive = true;
	std::vector<Step> Steps;
};

class RenderQueue
{
public:
	static void Add(Task task, size_t target);
	static void Execute();
	static void Reset();

	static RenderQueue& Get();

private:
	void AddImpl(Task task, size_t target);
	void ExecuteImpl();
	void ResetImpl();

private:
	RenderQueue() = default;
	RenderQueue(const RenderQueue&) = delete;
	RenderQueue& operator=(const RenderQueue&) = delete;

private:
	std::array<std::vector<Task>, 3> Stages;
};
