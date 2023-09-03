#pragma once

#include "Pass.h"
#include "Rendering/Utilities.h"
#include "RenderQueue.h"
#include "Rendering/Texture.h"
#include "Rendering/DepthCube.h"

class ResourcesPass : public Pass
{
protected:
	ResourcesPass(std::string&& name);
	ResourcesPass(std::string&& name,
				  GPUObjectBase&& resources);

	template<typename T, typename... Args>
	void Add(Args&&... args)
	{
		Resources.Add<T>(std::forward<Args>(args)...);
	}	
	
	virtual void Bind() const override;
	virtual void Validate() override;

protected:
	GPUObjectBase Resources;
};

class ClearPass : public Pass
{
public:
	ClearPass(std::string&& name);

	void Execute() const override;
};

class FullScreenPass : public ResourcesPass
{
public:
	FullScreenPass(std::string&& name);

	void Execute() const;
private:
	bool Initialized = false;
};

class RenderQueuePass : public ResourcesPass
{
public:
	RenderQueuePass(std::string&& name);
	RenderQueuePass(std::string&& name, GPUObjectBase&& resources);

	void PushBack(Task task);
	void Execute() const;
	void Reset();

protected:
	std::vector<Task> Tasks;
};

class PhongPass : public RenderQueuePass
{
public:
	PhongPass(std::string&& name);
	void Execute() const override;

private:
	SharedPtr<CubeTextureDepth> ShadowMap;
};

class OutlineDrawPass : public RenderQueuePass
{
public:
	OutlineDrawPass(std::string&& name);
};

class OutlineMaskPass : public RenderQueuePass
{
public:
	OutlineMaskPass(std::string&& name);
};

class BlurOutlineDrawPass : public RenderQueuePass
{
public:
	BlurOutlineDrawPass(std::string&& name, uint32_t width, uint32_t height);

	void Execute() const override;
};

class HorizontalBlurPass : public FullScreenPass
{
public:
	HorizontalBlurPass(std::string&& name, uint32_t width, uint32_t height);
	void Execute() const override;

private:
	SharedPtr<UniformPS<Kernel>> ConvKernel;
	SharedPtr<UniformPS<BOOL>> HorizontalFlag;
	SharedPtr<RenderTarget> BlurScratchIn;
};

class VerticalBlurPass : public FullScreenPass
{
public:
	VerticalBlurPass(std::string&& name);
	void Execute() const override;

private:
	SharedPtr<UniformPS<Kernel>> ConvKernel;
	SharedPtr<UniformPS<BOOL>> HorizontalFlag;
	SharedPtr<RenderTarget> BlurScratchIn;
};

class PointLight;

class ShadowMappingPass : public RenderQueuePass
{
public:
	ShadowMappingPass(std::string&& name, const PointLight* pointLight = nullptr);
	void Execute() const override;

	void SetLightSource(const PointLight* pointLight);
	void SetDepthBuffer(SharedPtr<DepthStencil> depthStencil) const;

private:
	SharedPtr<ShadowRasterizerState> ShadowRasterizer;
	const PointLight* LightSource;
	SharedPtr<CubeTextureDepth> DepthCube;

	mutable DirectX::XMMATRIX Projection;
	mutable DirectX::XMMATRIX View;
	mutable DirectX::XMMATRIX ViewProjection;

	UniquePtr<UniformVS<DirectX::XMMATRIX>>	 ViewUniform;
	UniquePtr<UniformVS<DirectX::XMMATRIX>>	 ViewProjectionUniform;

	std::array<DirectX::XMFLOAT3, 6> CameraUp;
	std::array<DirectX::XMFLOAT3, 6> CameraOrientation;

	static constexpr uint32_t DepthDim = 1080;
};

class SkyboxPass : public ResourcesPass
{
public:
	SkyboxPass(std::string&& name);

	void Execute() const override;

private:
	uint32_t Count;
};