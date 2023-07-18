#pragma once

#include "Actor.h"
#include "Rendering/Mesh.h"

class Model : public Actor {
public:
	Model();
	Model(const TransformationIntrinsics& intrinsics);

	virtual void Draw() override;
	virtual void Update() override;

	void GUI();
protected:

private:
	void Init();

	UniquePtr<Node> Root;
};