#pragma once

#include "Core/Core.h"
#include "Rendering/Utilities.h"
#include "Rendering/RenderTarget.h"

class DepthStencil;
class RenderTarget;
class PassOutputBase;

class PassInputBase
{
public:
	virtual ~PassInputBase() = default;

	const std::string& GetName() const noexcept { return Name; }
	const std::string& GetPassName() const noexcept { return PassName; }
	const std::string& GetOutputName() const noexcept { return OutputName; }

	void SetTarget(std::string passName, std::string outputName);

	virtual void Bind(PassOutputBase& out) = 0;
	virtual void Validate() const = 0;

protected:
	PassInputBase(std::string&& name);

private:
	std::string Name;
	std::string PassName;
	std::string OutputName;
};

template<typename T>
requires std::is_base_of_v<BufferResource, T>
class PassInput : public PassInputBase
{
public:
	PassInput(std::string&& name, SharedPtr<T>& target)
		:PassInputBase(std::move(name)), Target(target)
	{}

	virtual void Validate() const override
	{
		if (!IsLinked) throw std::runtime_error("Unlinked input");
	}

	virtual void Bind(PassOutputBase& out) override;

private:
	SharedPtr<T>& Target;
	bool IsLinked = false;
};

class PassOutputBase
{
public:
	virtual ~PassOutputBase() = default;

	const std::string& GetName() const noexcept { return Name; }
	virtual void Validate() const = 0;

	virtual SharedPtr<BufferResource> GetBufferResource() const = 0;
	
protected:
	PassOutputBase(std::string&& name);

private:
	std::string Name;
};

template<typename T>
requires std::is_base_of_v<BufferResource, T>
class PassOutput : public PassOutputBase
{
public:
	PassOutput(std::string&& name, SharedPtr<T>& resource)
		:PassOutputBase(std::move(name)), Resource(resource)
	{}

	virtual void Validate() const override
	{}

	virtual SharedPtr<BufferResource> GetBufferResource() const override
	{
		if (IsLinked)
			throw std::runtime_error("Resource " + GetName() + " bound twice");

		IsLinked = true;
		return Resource;
	}

private:
	SharedPtr<T>& Resource;
	mutable bool IsLinked = false;
};

template<typename T>
requires std::is_base_of_v<BufferResource, T>
void PassInput<T>::Bind(PassOutputBase& out)
{
	auto ptr = std::dynamic_pointer_cast<T>(out.GetBufferResource());
	if (!ptr)
		throw std::runtime_error("Binding input " + GetName() + " to output "
								 + GetPassName() + "." + GetOutputName() + "has incompatible type with output");

	Target = std::move(ptr);
	IsLinked = true;
}

class Pass
{
public:
	Pass(std::string&& name) noexcept;
	virtual ~Pass() = default;

	virtual void Execute() const = 0;
	virtual void Reset() {}
	const std::string& GetName() const noexcept { return Name; }
	const std::vector<UniquePtr<PassInputBase>>& GetInputs() const { return Inputs; }

	PassOutputBase& GetOutput(const std::string& name) const;
	PassInputBase& GetInput(const std::string& name) const;

	void SetInputSource(const std::string& inputName, const std::string& targetName);
	virtual void Validate();

protected:
	void Register(UniquePtr<PassInputBase> input);
	void Register(UniquePtr<PassOutputBase> output);

	template<typename T, typename... Args>
	requires std::is_constructible_v<T, Args...>
	void Register(Args&&... args)
	{
		auto ptr = MakeUnique<T>(std::forward<Args>(args)...);
		Register(std::move(ptr));
	}

	virtual void Bind() const;

protected:
	SharedPtr<DepthStencil> DStencil;
	SharedPtr<RenderTarget> RTarget;

private:
	std::vector<UniquePtr<PassInputBase>> Inputs;
	std::vector<UniquePtr<PassOutputBase>> Outputs;
	std::string Name;
};