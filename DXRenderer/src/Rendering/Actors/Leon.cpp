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
	:ActorBase(intrinsics)
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
	ActorBase<Model>::Update();
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

void Model::InitializeType()
{
	namespace WRL = Microsoft::WRL;

	if (IsInitialized())
		return;

	Root = Node::Build(*this, "Nanosuit\\nanosuit.obj");
}

void Model::Init()
{
	InitializeType();
	//Components.Add(MakeUnique<Material>(1));
}