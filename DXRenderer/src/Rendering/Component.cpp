#include "Component.h"

void ComponentGroup::Bind() const
{
	for (const auto& component : Components)
		component->Bind();
}

void ComponentGroup::Add(UniquePtr<Component> component)
{
	Components.emplace_back(std::move(component));
}

void ComponentGroup::Add(ComponentGroup componentGroup)
{
	for (auto& component : componentGroup.Components)
		Add(std::move(component));
}

ComponentGroup::ComponentGroup(ComponentGroup&& other) noexcept
{
	this->Components = std::move(other.Components);
}

ComponentGroup& ComponentGroup::operator=(ComponentGroup&& other) noexcept
{
	if (this != &other)
		Components = std::move(other.Components);

	return *this;
}