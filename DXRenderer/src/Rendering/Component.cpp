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
