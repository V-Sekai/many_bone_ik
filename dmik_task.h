#pragma once
#include "bone_chain_item.h"

class DMIKTask : public Reference {
	GDCLASS(DMIKTask, Reference);

protected:
	static void _bind_methods() {}

public:
	Skeleton3D *skeleton = nullptr;

	Ref<BoneChainItem> chain = memnew(BoneChainItem);
	// Settings
	float min_distance = 0.01f;
	int iterations = 4;
	int max_iterations = 1.0f;
	// dampening dampening angle in radians.
	// Set this to -1 if you want to use the armature's default.
	float dampening = 0.05f;
	// stabilizing_passes number of stabilization passes to run.
	// Set this to -1 if you want to use the armature's default.
	int stabilizing_passes = -1;

	// Bone data
	int root_bone = -1;
	Vector<Ref<BoneEffectorTransform>> end_effectors;
	Ref<SkeletonModification3DDMIK> dmik;
};

