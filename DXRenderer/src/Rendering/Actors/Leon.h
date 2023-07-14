#pragma once

#include "Actor.h"
#include "Rendering/Mesh.h"

class Model : public ActorBase<Model> {
public:
	Model();
	Model(const TransformationIntrinsics& intrinsics);

	virtual void Draw() override;
	virtual void Update() override;

	void GUI();
protected:
	void InitializeType() override;

private:
	void Init();

	UniquePtr<Node> Root;
};