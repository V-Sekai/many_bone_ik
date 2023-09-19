/**************************************************************************/
/*  skeleton_profile_humanoid_constraint.h                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "scene/resources/skeleton_profile.h"

class SkeletonProfileHumanoidConstraint : public SkeletonProfileHumanoid {
	GDCLASS(SkeletonProfileHumanoidConstraint, SkeletonProfileHumanoid);

public:
	// Unit vector pointing towards the left side of imported 3D assets.
	const Vector3 MODEL_LEFT = Vector3(1, 0, 0);

	// Unit vector pointing towards the right side of imported 3D assets.
	const Vector3 MODEL_RIGHT = Vector3(-1, 0, 0);

	// Unit vector pointing towards the top side (up) of imported 3D assets.
	const Vector3 MODEL_TOP = Vector3(0, 1, 0);

	// Unit vector pointing towards the bottom side (down) of imported 3D assets.
	const Vector3 MODEL_BOTTOM = Vector3(0, -1, 0);

	// Unit vector pointing towards the front side (facing forward) of imported 3D assets.
	const Vector3 MODEL_FRONT = Vector3(0, 0, 1);

	// Unit vector pointing towards the rear side (facing backwards) of imported 3D assets.
	const Vector3 MODEL_REAR = Vector3(0, 0, -1);

	struct LimitCone {
		Vector3 center = Vector3(0, 1, 0);
		float radius = Math_PI;

		LimitCone(Vector3 p_center = Vector3(), float p_radius = 0) :
				center(p_center), radius(p_radius) {}
	};

	struct BoneConstraint {
		float twist_from = Math_TAU;
		float twist_range = Math_PI;
		Vector<LimitCone> swing_limit_cones;
		float resistance = 0;

		BoneConstraint(float p_twist_from = 0, float p_twist_range = 0, Vector<LimitCone> p_swing_limit_cones = Vector<LimitCone>(), float p_resistance = 0) :
				twist_from(p_twist_from), twist_range(p_twist_range), swing_limit_cones(p_swing_limit_cones), resistance(p_resistance) {
			p_swing_limit_cones.resize(10);
		}
		BoneConstraint(const BoneConstraint &) = default;
		BoneConstraint(BoneConstraint &&) = default;
		BoneConstraint &operator=(const BoneConstraint &) = default;
		BoneConstraint &operator=(BoneConstraint &&) = default;
		explicit BoneConstraint(Vector<LimitCone> p_swing_limit_cones) :
				swing_limit_cones(std::move(p_swing_limit_cones)) {}
	};
	void set_bone_constraint(const StringName &p_bone_name, float p_twist_from, float p_twist_range, Vector<LimitCone> p_swing_limit_cones, float p_resistance);
	BoneConstraint get_bone_constraint(const StringName &p_bone_name) const;
	SkeletonProfileHumanoidConstraint();
	~SkeletonProfileHumanoidConstraint() {
	}

private:
	HashMap<StringName, BoneConstraint> bone_constraints;
};
