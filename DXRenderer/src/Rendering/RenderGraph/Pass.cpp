#include "Pass.h"

#include "Rendering/RenderTarget.h"

#include <stdexcept>
#include <algorithm>

void PassInputBase::SetTarget(std::string passName, std::string outputName)
{
	if (passName.empty())
		throw std::invalid_argument("Empty pass name");

	const bool passNameValid = std::all_of(passName.begin(), passName.end(), [](char c)
											{
												return std::isalnum(c) || c == '_';
											});
	if (passName != "$" && (!passNameValid || std::isdigit(passName.front())))
		throw std::invalid_argument("Invalid name - unsupported characters");

	if (outputName.empty())
		throw std::invalid_argument("Empty output name");

	const bool outputNameValid = std::all_of(outputName.begin(), outputName.end(), [](char c)
											{
												return std::isalnum(c) || c == '_';
											});
	if (!outputNameValid || std::isdigit(outputName.front()))
		throw std::invalid_argument("Invalid name - unsupported characters");

	PassName = passName;
	OutputName = outputName;
}

PassInputBase::PassInputBase(std::string&& name)
	:Name(std::move(name))
{
	if (Name.empty())
		throw std::invalid_argument("Invalid name - Empty pass name");
	const bool nameCharsValid = std::all_of(Name.begin(), Name.end(), [](char c)
											{
												return std::isalnum(c) || c == '_';
											});
	if (!nameCharsValid || std::isdigit(Name.front()))
		throw std::invalid_argument("Invalid name - unsupported characters");
}

Pass::Pass(std::string&& name) noexcept
	:Name(std::move(name))
{}

PassOutputBase& Pass::GetOutput(const std::string & name) const
{
	for (auto& out : Outputs)
		if (out->GetName() == name)
			return *out;

	throw std::range_error("Element not found among possible Outputs");
}

PassInputBase& Pass::GetInput(const std::string& name) const
{
	for (auto& in : Inputs)
		if (in->GetName() == name)
			return *in;

	throw std::range_error("Element not found among possible Inputs");
}

void Pass::Register(UniquePtr<PassOutputBase> output)
{
	auto it = std::find_if(Outputs.begin(), Outputs.end(),
						   [&output](const UniquePtr<PassOutputBase>& in)
						   {
							   return in->GetName() == output->GetName();
						   });

	if (it != Outputs.end()) throw std::invalid_argument("Registered output in conflict with existing registered input");
	Outputs.emplace_back(std::move(output));
}

inline void Pass::Bind() const
{
	if (RTarget)
		RTarget->Bind(*DStencil.get());
	else
		DStencil->Bind();
}

void Pass::SetInputSource(const std::string& inputName, const std::string& targetName)
{
	auto& input = GetInput(inputName);
	auto targetNames = SplitString(targetName, ".");

	if (targetNames.size() != 2)
		throw std::invalid_argument("Invalid target name");

	input.SetTarget(std::move(targetNames[0]), std::move(targetNames[1]));
}

void Pass::Validate()
{
	for (auto& in : Inputs)
		in->Validate();
	for (auto& out : Outputs)
		out->Validate();
}

void Pass::Register(UniquePtr<PassInputBase> input)
{
	auto it = std::find_if(Inputs.begin(), Inputs.end(),
						   [&input](const UniquePtr<PassInputBase>& in)
						   {
							   return in->GetName() == input->GetName();
						   });

	if (it != Inputs.end()) throw std::invalid_argument("Registered input in conflict with existing registered input");
	Inputs.emplace_back(std::move(input));
}

PassOutputBase::PassOutputBase(std::string&& name)
	:Name(std::move(name))
{}
