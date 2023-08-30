#pragma once

#include "Actor.h"
#include "Rendering/Mesh.h"
#include "Rendering/Node.h"

class Model : public Actor 
{
public:
	Model(const std::string& filename);
	Model(const std::string& filename, const TransformationIntrinsics& intrinsics);

	virtual void Submit(size_t channelsIn) override;
	virtual void Tick(float delta) override;

	virtual void GUI() override;
	const std::string& GetPath() const { return Path; }
	virtual void LinkTechniques() override;
protected:

private:
	void Init(const std::string& filename);

	UniquePtr<Node> Root;
	std::string Path;
};