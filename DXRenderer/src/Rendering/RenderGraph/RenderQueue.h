#pragma once

#include "Rendering/Buffer.h"
#include "Core/Core.h"
#include "Rendering/Graphics.h"
#include "Rendering/State.h"
#include "Rendering/Utilities.h"

#include <vector>

class Step
{
public:
	Step(std::string name);

	template<typename T, typename... Args>
	void Add(Args&&... args)
	{
		Resources.Add<T>(std::forward<Args>(args)...);
	}

	void Bind() const;
	void Submit(const GPUObject& renderObject) const;
	void Link();

private:
	std::string TargetPassName;
	GPUObject Resources;
	class RenderQueuePass* TargetPass{ nullptr };
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

	void Link();

private:
	bool IsActive = true;
	std::vector<Step> Steps;
};
