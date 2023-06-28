#pragma once

#include "Actor.h"
#include "Rendering/Mesh.h"

class Leon : public ActorBase<Leon> {
public:
	Leon();
	Leon(const TransformationIntrinsics& intrinsics);

	virtual void Draw() override;
	virtual void Update() override;
protected:
	void InitializeType() override;

private:
	void Init();

	UniquePtr<Node> Root;
};