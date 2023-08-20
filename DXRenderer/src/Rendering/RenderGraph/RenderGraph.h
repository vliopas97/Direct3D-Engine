#pragma once

#include "Rendering/Utilities.h"

class Pass;
class PassInputBase;
class PassOutputBase;
class RenderQueuePass;
class DepthStencil;
class RenderTarget;

class RenderGraph
{
public:
	static RenderGraph& Get();
	static void SetInputTarget(const std::string& name, const std::string& target);
	static void Execute();
	static void Reset();
	static void Add(UniquePtr<Pass> pass);
	static void LinkInputs(Pass& pass);
	static void LinkGlobalInputs();
	static void Validate();
	static RenderQueuePass& GetRenderQueue(const std::string& passName);

	static void AddGlobalInputs(UniquePtr<PassInputBase> in);
	static void AddGlobalOutputs(UniquePtr<PassOutputBase> out);

private:
	RenderGraph();
	~RenderGraph() = default;

	void SetInputTargetImpl(const std::string& name, const std::string& target);
	void ExecuteImpl();
	void ResetImpl();
	void AddImpl(UniquePtr<Pass> pass);
	void LinkInputsImpl(Pass& pass);
	void LinkGlobalInputsImpl();
	void ValidateImpl();
	RenderQueuePass& GetRenderQueueImpl(const std::string& passName);


	void AddGlobalInputsImpl(UniquePtr<PassInputBase> in);
	void AddGlobalOutputsImpl(UniquePtr<PassOutputBase> out);

private:
	std::vector<UniquePtr<Pass>> Passes;
	std::vector<UniquePtr<PassInputBase>> GlobalInputs;
	std::vector<UniquePtr<PassOutputBase>> GlobalOutputs;
	SharedPtr<DepthStencil> DepthBuffer;
	SharedPtr<RenderTarget> BackBuffer;
	bool IsValidated = false;
};