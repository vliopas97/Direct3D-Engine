#include "RenderQueue.h"

Step::Step(size_t pass)
	:PassNumber(pass), Resources{}
{}

void Step::Bind() const
{
	Resources.Bind();
}

void Step::Submit(const GPUObject& renderObject) const
{
	RenderQueue::Add(Task{ &renderObject, this }, PassNumber);
}

void RenderQueue::AddImpl(Task task, size_t target)
{
	Stages[target].push_back(task);
}

void RenderQueue::Execute() { RenderQueue::Get().ExecuteImpl(); }

void RenderQueue::ExecuteImpl()
{
	StencilState<> StencilOff{};
	StencilState<DepthStencilMode::Write > StencilWrite{};
	StencilState<DepthStencilMode::Mask > StencilMask{};
	BlendState blend("default", false);
	NullPixelShader n{};

	DStencil.Clear();
	FullScreenFilter.RT1.Clear();
	CurrentGraphicsContext::GraphicsInfo->SwapBuffers(DStencil);

	StencilOff.Bind();
	blend.Bind();
	for (auto& t : Stages[0])
		t.Execute();

	StencilWrite.Bind();
	n.Bind();
	for (auto& t : Stages[1])
		t.Execute();

	StencilOff.Bind();
	FullScreenFilter.RT1.Bind();
	for (auto& t : Stages[2])
		t.Execute();
	FullScreenFilter.RT2.Bind();
	FullScreenFilter.RT1.BindAsTexture();
	FullScreenFilter.DrawHorizontal();
	CurrentGraphicsContext::GraphicsInfo->SwapBuffers(DStencil);
	StencilMask.Bind();
	FullScreenFilter.DrawVertical();
}

void RenderQueue::Reset() { RenderQueue::Get().ResetImpl(); }

void RenderQueue::ResetImpl()
{
	for (auto& s : Stages)
		s.clear();
}

inline RenderQueue::RenderQueue()
	:DStencil(CurrentGraphicsContext::GraphicsInfo->GetWidth(), CurrentGraphicsContext::GraphicsInfo->GetHeight()),
	FullScreenFilter(CurrentGraphicsContext::GraphicsInfo->GetWidth() / 2,
					 CurrentGraphicsContext::GraphicsInfo->GetHeight() / 2,
					 10, 4.5f)
{}

void RenderQueue::Add(Task task, size_t target) { RenderQueue::Get().AddImpl(task, target); }

RenderQueue& RenderQueue::Get()
{
	static RenderQueue singleton;
	return singleton;
}

void Technique::Submit(const GPUObject& renderObject)
{
	if (!IsActive) return;

	for (const auto& step : Steps)
		step.Submit(renderObject);
}

void Technique::PushBack(Step&& step)
{
	Steps.emplace_back(std::move(step));
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