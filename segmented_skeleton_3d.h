#pragma once

#include "core/object/reference.h"
#include "shadow_skeleton_bone.h"
#include "skeleton_modification_3d_ewbik.h"

class EWBIKSegmentedSkeleton3DNew : public EWBIKSegmentedSkeleton3D {
	GDCLASS(EWBIKSegmentedSkeleton3DNew, EWBIKSegmentedSkeleton3D);
public:

protected:
	static void _bind_methods() {}
};