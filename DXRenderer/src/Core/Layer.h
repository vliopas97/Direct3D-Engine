#pragma once

#include "Core\Core.h"

#include <d3d11.h>

class Layer
{
public:
	virtual void OnAttach() {};
	virtual void OnDetach() {};
	virtual void Render() {};
	virtual void OnEvent(Event& e) {};
};

class ImGuiLayer : public Layer
{
public:
	ImGuiLayer() = default;
	~ImGuiLayer() = default;

	void OnAttach() override;
	void Render() override;
	void OnDetach() override;
	void OnEvent(Event& e) override;

	void Begin();
	void End();
};