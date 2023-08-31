#include "RenderGraph.h"

#include "Pass.h"
#include "PassExtensions.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering/Graphics.h"
#include "Rendering/RenderTarget.h"

RenderGraph& RenderGraph::Get()
{
	static RenderGraph singleton;
	return singleton;
}

void RenderGraph::SetInputTarget(const std::string& name, const std::string& target)
{
	RenderGraph::Get().SetInputTargetImpl(name, target);
}

void RenderGraph::Execute()
{
	RenderGraph::Get().ExecuteImpl();
}

void RenderGraph::Reset()
{
	RenderGraph::Get().ResetImpl();
}

void RenderGraph::Add(UniquePtr<Pass> pass)
{
	RenderGraph::Get().AddImpl(std::move(pass));
}

void RenderGraph::LinkInputs(Pass& pass)
{
	RenderGraph::Get().LinkInputsImpl(pass);
}

void RenderGraph::LinkGlobalInputs()
{
	RenderGraph::Get().LinkGlobalInputsImpl();
}

void RenderGraph::Validate()
{
	RenderGraph::Get().ValidateImpl();
}

RenderQueuePass& RenderGraph::GetRenderQueue(const std::string& passName)
{
	return RenderGraph::Get().GetRenderQueueImpl(passName);
}

void RenderGraph::AddGlobalInputs(UniquePtr<PassInputBase> in)
{
	RenderGraph::Get().AddGlobalInputsImpl(std::move(in));
}

void RenderGraph::AddGlobalOutputs(UniquePtr<PassOutputBase> out)
{
	RenderGraph::Get().AddGlobalOutputsImpl(std::move(out));
}

RenderGraph::RenderGraph()
	:BackBuffer(CurrentGraphicsContext::GraphicsInfo->GetTarget()),
	DepthBuffer(MakeShared<DepthStencilOutput>(CurrentGraphicsContext::GraphicsInfo->GetWidth(),
											   CurrentGraphicsContext::GraphicsInfo->GetHeight())),
	ShadowRasterizer(MakeShared<ShadowRasterizerState>())
{
	GlobalOutputs.emplace_back(MakeUnique<PassOutput<RenderTarget>>("backBuffer", BackBuffer));
	GlobalOutputs.emplace_back(MakeUnique<PassOutput<DepthStencil>>("depthBuffer", DepthBuffer));
	GlobalOutputs.emplace_back(MakeUnique <PassOutput<ShadowRasterizerState>>("shadowRasterizer", ShadowRasterizer));
	GlobalInputs.emplace_back(MakeUnique<PassInput<RenderTarget>>("backBuffer", BackBuffer));

	//--------------------------

	{
		auto pass = MakeUnique<ClearPass>("clear");
		pass->SetInputSource("renderTarget", "$.backBuffer");
		pass->SetInputSource("depthStencil", "$.depthBuffer");
		AddImpl(std::move(pass));
	}
	{
		auto pass = MakeUnique<ShadowMappingPass>("shadowMap");
		pass->SetInputSource("shadowRasterizer", "$.shadowRasterizer");
		AddImpl(std::move(pass));
	}
	{
		auto pass = MakeUnique<PhongPass>("phong");
		pass->SetInputSource("shadowMap", "shadowMap.map");
		pass->SetInputSource("renderTarget", "clear.renderTarget");
		pass->SetInputSource("depthStencil", "clear.depthStencil");
		AddImpl(std::move(pass));
	}
	{
		auto pass = MakeUnique<SkyboxPass>("skybox");
		pass->SetInputSource("renderTarget", "phong.renderTarget");
		pass->SetInputSource("depthStencil", "phong.depthStencil");
		AddImpl(std::move(pass));
	}
	{
		auto pass = MakeUnique<OutlineMaskPass>("outlineMask");
		pass->SetInputSource("depthStencil", "skybox.depthStencil");
		AddImpl(std::move(pass));
	}
	{
		auto pass = MakeUnique<OutlineDrawPass>("outlineDraw");
		pass->SetInputSource("renderTarget", "skybox.renderTarget");
		pass->SetInputSource("depthStencil", "outlineMask.depthStencil");
		AddImpl(std::move(pass));
	}
	SetInputTargetImpl("backBuffer", "outlineDraw.renderTarget");
	ValidateImpl();
}

void RenderGraph::SetInputTargetImpl(const std::string& name, const std::string& target)
{
	auto it = std::find_if(GlobalInputs.begin(), GlobalInputs.end(),
						   [&name](const UniquePtr<PassInputBase>& ptr)
						   {
							   return ptr->GetName() == name;
						   });

	if (it == GlobalInputs.end()) throw std::invalid_argument("Name invalid - No such input exists");

	auto split = SplitString(target, ".");
	if (split.size() != 2) throw std::invalid_argument("Name invalid - Incorrect name form for global buffers");

	(*it)->SetTarget(split[0], split[1]);
}

void RenderGraph::ExecuteImpl()
{
	ASSERT(IsValidated);

	for (const auto& pass : Passes)
		pass->Execute();
}

void RenderGraph::ResetImpl()
{
	ASSERT(IsValidated);
	for (auto& pass : Passes)
		pass->Reset();
}

void RenderGraph::AddImpl(UniquePtr<Pass> pass)
{
	ASSERT(!IsValidated);

	auto it = std::find_if(Passes.begin(), Passes.end(),
						   [&pass](const auto& ptr)
						   {
							   return pass->GetName() == ptr->GetName();
						   });

	if (it != Passes.end()) throw std::invalid_argument("Pass name already exists");

	LinkInputsImpl(*pass);
	Passes.emplace_back(std::move(pass));
}

void RenderGraph::LinkInputsImpl(Pass& pass)
{
	for (auto& in : pass.GetInputs())
	{
		const std::string& name = in->GetPassName();

		if (name == "$")
		{
			bool bound = false;
			for (auto& out : GlobalOutputs)
			{
				if (out->GetName() == in->GetOutputName())
				{
					in->Bind(*out);
					bound = true;
					break;
				}
			}

			if (!bound)
				throw std::runtime_error("Output " + in->GetOutputName() + " not found in globals");
		}
		else
		{
			auto it = std::find_if(Passes.begin(), Passes.end(),
								   [&name](const auto& ptr)
								   {
									   return name == ptr->GetName();
								   });

			if (it == Passes.end()) throw std::runtime_error("Pass input not found among existing Passes");

			auto& out = (*it)->GetOutput(in->GetOutputName());
			in->Bind(out);
		}
	}
}

void RenderGraph::LinkGlobalInputsImpl()
{
	for (auto& in : GlobalInputs)
	{
		const std::string& name = in->GetPassName();
		auto it = std::find_if(Passes.begin(), Passes.end(),
							   [&name](const auto& ptr)
							   {
								   return name == ptr->GetName();
							   });

		if (it == Passes.end()) throw std::runtime_error("Pass input not found among existing Passes");

		auto& out = (*it)->GetOutput(in->GetOutputName());
		in->Bind(out);
	}
}

void RenderGraph::ValidateImpl()
{
	ASSERT(!IsValidated);
	for (const auto& pass : Passes)
		pass->Validate();

	LinkGlobalInputsImpl();
	IsValidated = true;
}

RenderQueuePass& RenderGraph::GetRenderQueueImpl(const std::string& passName)
{
	try
	{
		for (const auto& pass : Passes)
			if (pass->GetName() == passName)
				return dynamic_cast<RenderQueuePass&>(*pass);
	}
	catch (std::bad_cast&)
	{
		throw std::invalid_argument("Cannot find a RenderQueuePass with such name");
	}
}

void RenderGraph::AddGlobalInputsImpl(UniquePtr<PassInputBase> in)
{
	GlobalInputs.emplace_back(std::move(in));
}

void RenderGraph::AddGlobalOutputsImpl(UniquePtr<PassOutputBase> out)
{
	GlobalOutputs.emplace_back(std::move(out));
}