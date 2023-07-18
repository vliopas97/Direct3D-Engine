#pragma once

#include "Actor.h"
#include "Rendering/Mesh.h"

class Model : public Actor {
public:
	Model(const std::string& filename);
	Model(const std::string& filename, const TransformationIntrinsics& intrinsics);

	virtual void Draw() override;
	virtual void Update() override;

	void GUI();
	const std::string& GetPath() const { return Path; }
protected:

private:
	void Init(const std::string& filename);

	UniquePtr<Node> Root;
	std::string Path;
};