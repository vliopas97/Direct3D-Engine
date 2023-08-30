#include "RenderQueue.h"
#include "RenderGraph.h"
#include "PassExtensions.h"

Step::Step(std::string name)
	:TargetPassName(std::move(name)), Resources{}
{}

void Step::Bind() const
{
	Resources.Bind();
}

void Step::Submit(const GPUObject& renderObject) const
{
	TargetPass->PushBack(Task{ &renderObject, this });
}

void Step::Link()
{
	ASSERT(TargetPass == nullptr);
	TargetPass = &RenderGraph::GetRenderQueue(TargetPassName);
}

Task::Task(const GPUObject* renderObject, const Step* step)
	:RenderObject(renderObject), TStep(step)
{}

void Task::Execute() const
{
	RenderObject->Bind();
	TStep->Bind();
	CurrentGraphicsContext::Context()->DrawIndexed(RenderObject->GetIndexBuffer()->GetCount(), 0, 0);
}

Technique::Technique(size_t channels)
	:Channels(channels)
{}

void Technique::Submit(const GPUObject& renderObject, size_t channelsIn)
{
	if (!IsActive || (Channels & channelsIn) == 0) return;

	for (const auto& step : Steps)
		step.Submit(renderObject);
}

void Technique::PushBack(Step&& step)
{
	Steps.emplace_back(std::move(step));
}

void Technique::Link()
{
	for (auto& step : Steps)
		step.Link();
}