#pragma once

#include "Core\Core.h"

class Component
{
public:
	virtual void Bind() const {};
	virtual ~Component() = default;
};

class ComponentGroup : public Component
{
public:
	void Bind() const override;

	void Add(UniquePtr<Component> component);
	void Add(ComponentGroup componentGroup);

	ComponentGroup() = default;
	ComponentGroup(ComponentGroup&& other) noexcept;
	ComponentGroup& operator=(ComponentGroup&& other) noexcept;

private:
	std::vector<UniquePtr<Component>> Components;
};