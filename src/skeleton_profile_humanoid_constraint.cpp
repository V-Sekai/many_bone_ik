#include "skeleton_profile_humanoid_constraint.h"

SkeletonProfileHumanoidConstraint::SkeletonProfileHumanoidConstraint() {
	Vector<StringName> bone_names = { "Spine", "Chest", "UpperChest", "Hips", "Neck", "Head", "LeftUpperLeg", "RightUpperLeg", "LeftLowerLeg", "RightLowerLeg", "LeftFoot", "RightFoot", "LeftShoulder", "RightShoulder", "LeftUpperArm", "RightUpperArm", "LeftLowerArm", "RightLowerArm", "LeftHand", "RightHand", "LeftThumb", "RightThumb", "LeftEye", "RightEye" };
	for (int i = 0; i < bone_names.size(); ++i) {
		StringName bone_name = bone_names[i];
		Vector<LimitCone> swing_limit_cones;
		int32_t bone_i = find_bone(bone_name);
		if (bone_i == -1) {
			continue;
		}
		Transform3D reference_pose = get_reference_pose(bone_i);
		Vector3 y_up = reference_pose.basis.get_column(Vector3::AXIS_Y).normalized();
		Vector3 y_up_backwards = y_up;
		y_up_backwards.y = -y_up_backwards.y;
		float twist_range = Math::deg_to_rad(180.0f);
		float twist_from = reference_pose.basis.get_euler().y;
		float resistance = 0;
		if (bone_name == "Hips") {
			twist_from = Math::deg_to_rad(0.0f);
			twist_range = Math::deg_to_rad(40.0f);
			resistance = 0.5f;
		} else if (bone_name == "Spine") {
			twist_from = Math::deg_to_rad(4.0f);
			twist_range = Math::deg_to_rad(4.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(3.0f)));
			resistance = 0.5f;
		} else if (bone_name == "Chest") {
			twist_from = Math::deg_to_rad(5.0f);
			twist_range = Math::deg_to_rad(-10.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(3.0f)));
			resistance = 0.5f;
		} else if (bone_name == "UpperChest") {
			twist_from = Math::deg_to_rad(10.0f);
			twist_range = Math::deg_to_rad(40.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(10.0f)));
			resistance = 0.6f;
		} else if (bone_name == "Neck") {
			twist_from = Math::deg_to_rad(15.0f);
			twist_range = Math::deg_to_rad(15.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(10.0f)));
			resistance = 0.6f;
		} else if (bone_name == "Head") {
			twist_from = Math::deg_to_rad(15.0f);
			twist_range = Math::deg_to_rad(15.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(15.0f)));
			resistance = 0.7f;
		} else if (bone_name == "LeftUpperLeg") {
			twist_from = Math::deg_to_rad(300.0f);
			twist_range = Math::deg_to_rad(10.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(25.0f)));
			resistance = 0.8f;
		} else if (bone_name == "LeftLowerLeg") {
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(MODEL_REAR, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(y_up_backwards, Math::deg_to_rad(2.5f)));
		} else if (bone_name == "RightUpperLeg") {
			twist_from = Math::deg_to_rad(300.0f);
			twist_range = Math::deg_to_rad(10.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(25.0f)));
			resistance = 0.8f;
		} else if ( bone_name == "RightLowerLeg") {
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(MODEL_REAR, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(y_up_backwards, Math::deg_to_rad(2.5f)));
		} else if (bone_name == "LeftFoot" || bone_name == "RightFoot") {
			swing_limit_cones.push_back(LimitCone(MODEL_BOTTOM, Math::deg_to_rad(5.0f)));
		} else if (bone_name == "LeftShoulder" || bone_name == "RightShoulder") {
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(30.0f)));
		} else if (bone_name == "LeftUpperArm" || bone_name == "RightUpperArm") {
			twist_from = Math::deg_to_rad(80.0f);
			twist_range = Math::deg_to_rad(12.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(90.0f)));
			resistance = 0.3;
		} else if (bone_name == "LeftLowerArm") {
			twist_from = Math::deg_to_rad(-55.0f);
			twist_range = Math::deg_to_rad(50.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(MODEL_FRONT, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(y_up_backwards, Math::deg_to_rad(2.5f)));
			resistance = 0.4;
		} else if (bone_name == "RightLowerArm") {
			twist_from = Math::deg_to_rad(-145.0f);
			twist_range = Math::deg_to_rad(50.0f);
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(MODEL_FRONT, Math::deg_to_rad(2.5f)));
			swing_limit_cones.push_back(LimitCone(y_up_backwards, Math::deg_to_rad(2.5f)));
			resistance = 0.4;
		} else if (bone_name == "LeftHand" || bone_name == "RightHand") {
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(60.0f)));
		} else if (bone_name == "LeftThumb" || bone_name == "RightThumb") {
			swing_limit_cones.push_back(LimitCone(y_up, Math::deg_to_rad(90.0f)));
		}
		set_bone_constraint(bone_name, twist_from, twist_range, swing_limit_cones, resistance);
	}
}

SkeletonProfileHumanoidConstraint::BoneConstraint SkeletonProfileHumanoidConstraint::get_bone_constraint(const StringName &p_bone_name) const {
	if (bone_constraints.has(p_bone_name)) {
		return bone_constraints[p_bone_name];
	} else {
		return BoneConstraint();
	}
}

void SkeletonProfileHumanoidConstraint::set_bone_constraint(const StringName &p_bone_name, float p_twist_from, float p_twist_range, Vector<LimitCone> p_swing_limit_cones, float p_resistance = 0.0f) {
	bone_constraints[p_bone_name] = BoneConstraint(p_twist_from, p_twist_range, p_swing_limit_cones, p_resistance);
}
