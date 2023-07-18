#include "Leon.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering\Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <imgui.h>


Model::Model()
{
	Init();
}

Model::Model(const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init();
}

void Model::Draw()
{
	Root->Draw();
	Components.Bind();
}

void Model::Update()
{
	Actor::Update();
	Root->SetTransform(Transform.GetMatrix());
	Root->Update();
}

void Model::GUI()
{
	if (ImGui::Begin("Leon"))
	{
		ImGui::Columns(2, nullptr, true);

		Root->ShowTree();

	}
	ImGui::End();
}

void Model::Init()
{
	Root = Node::Build(*this, "Nanosuit\\nanosuit.obj");
	//Components.Add(MakeUnique<Material>(1));
}