#include "RenderQueue.h"

Step::Step(size_t pass)
	:PassNumber(pass), Resources{}
{}

void Step::Bind() const
{
	Resources.Bind();
}

// TO DO SUBMIT

void Step::Submit(const GPUObject& renderObject) const
{
	RenderQueue::Get().Add(Task{ &renderObject, this }, PassNumber);
}

void RenderQueue::Add(Task task, size_t target)
{
	Stages[target].push_back(task);
}

void RenderQueue::Execute()
{
	Resources[0].Add<StencilState<>>("RenderQueueStencil0");
	Resources[0].Bind();

	Resources[1].Add<StencilState<DepthStencilMode::Write>>("RenderQueueStencil1");
	Resources[1].Add<NullPixelShader>();
	Resources[1].Bind();

	Resources[2].Add<StencilState<DepthStencilMode::Mask>>("RenderQueueStencil1");
	Resources[2].Bind();
	// TODO MOVE SOLID COLOR BUFFER IN HERE
}

void RenderQueue::Reset()
{
	for (auto& s : Stages)
		s.clear();
}

RenderQueue& RenderQueue::Get() 
{
	static RenderQueue singleton;
	return singleton;
}

// TO DO SUBMIT

inline void Technique::Submit(const GPUObject& renderObject)
{
	if (!IsActive) return;

	for (const auto& step : Steps)
		step.Submit(renderObject);
}
