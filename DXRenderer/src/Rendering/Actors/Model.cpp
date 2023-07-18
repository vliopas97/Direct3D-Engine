#include "Model.h"
#include "Rendering/CurrentGraphicsContext.h"
#include "Rendering\Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <imgui.h>


Model::Model(const std::string& filename)
{
	Init(filename);
}

Model::Model(const std::string& filename, const TransformationIntrinsics& intrinsics)
	:Actor(intrinsics)
{
	Init(filename);
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

void Model::Init(const std::string& filename)
{
	Path = "\\Model\\" + filename;
	Path = Path.substr(0, Path.find_last_of("\\/") + 1);
	Root = Node::Build(*this, filename);

}