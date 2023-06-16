#pragma once

#include "Core\Core.h"

class Component
{
public:
	virtual void Bind() const = 0;
};

class ComponentGroup : public Component
{
public:
	void Bind() const override;

	void Add(UniquePtr<Component> component);

	void Add(ComponentGroup componentGroup);

private:
	std::vector<UniquePtr<Component>> Components;
};