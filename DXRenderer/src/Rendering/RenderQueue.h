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
	Task(const GPUObject* renderObject, const Step* step)
		:RenderObject(renderObject), TStep(step)
	{}

	void Execute() const
	{
		RenderObject->Bind();
		TStep->Bind();
		CurrentGraphicsContext::Context()->DrawIndexed(RenderObject->GetIndexBuffer()->GetCount(), 0, 0);
	}

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

private:
	bool IsActive = true;
	std::vector<Step> Steps;
};

class RenderQueue
{
public:
	void Add(Task task, size_t target);
	void Execute();
	void Reset();

	static RenderQueue& Get();

private:
	RenderQueue() = default;
	RenderQueue(const RenderQueue&) = delete;
	RenderQueue& operator=(const RenderQueue&) = delete;

private:
	std::array<std::vector<Task>, 3> Stages;
	std::array<GPUObject, 3> Resources;
};
