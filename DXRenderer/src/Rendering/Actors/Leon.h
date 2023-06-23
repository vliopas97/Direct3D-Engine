#pragma once

#include "Actor.h"

class Leon : public ActorBase<Leon> {
public:
	Leon();
	Leon(const TransformationIntrinsics& intrinsics);

	virtual void Update() override;

protected:
	void InitializeType() override;

private:
	void Init();
};