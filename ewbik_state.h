#pragma once

#include "kusudama_constraint.h"
#include "ewbik_transform.h"
#include "shadow_bone_3d.h"

#include "core/io/resource.h"
#include "scene/3d/skeleton_3d.h"
#include "ik_node_3d.h"

class ShadowBone3D;
class SkeletonModification3DEWBIK;
class IKBasis;
class IKNode3D;

// Skeleton data structure
class EWBIKState : public Resource {
	GDCLASS(EWBIKState, Resource);
	friend class SkeletonModification3DEWBIK;

	Ref<SkeletonModification3DEWBIK> mod;
	Skeleton3D *skeleton = nullptr;
	int bone_count = 0;

	Vector<ShadowBone3D> bones;
	Vector<int32_t> parentless_bones;

public:
	void set_pain(int p_bone, const float &p_pain);
	float get_pain(int p_bone) const;
	bool get_springy(int32_t p_bone) const;
	void set_springy(int32_t p_bone, bool p_springy);
	float get_cos_half_dampen(int32_t p_bone) const;
	void set_cos_half_dampen(int32_t p_bone, float p_cos_half_dampen);
	Vector<float> get_cos_half_returnful_dampened(int32_t p_bone) const;
	Vector<float> get_half_returnful_dampened(int32_t p_bone) const;
	void set_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened);
	void set_cos_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened);
	IKNode3D global_constraint_pose_to_local_pose(int p_bone_idx, IKNode3D p_global_pose);
	IKNode3D global_shadow_pose_to_local_pose(int p_bone_idx, IKNode3D p_global_pose);
	void force_update_bone_children_transforms(int p_bone_idx);
	void update_skeleton();
	void mark_dirty(int32_t p_bone);
	bool is_dirty(int32_t p_bone) const;
	void _update_process_order();
	void rotate_by(int32_t p_bone, Quat p_add_rotation);
	void rotate_to(int32_t p_bone, Quat p_rot);
	void translate_to(int32_t p_bone, Vector3 p_target);
	Ray get_ray_x(int32_t p_bone);
	Ray get_ray_y(int32_t p_bone);
	Ray get_ray_z(int32_t p_bone);
	void rotate_about_x(int32_t p_bone, float angle);
	void rotate_about_y(int32_t p_bone, float angle);
	void rotate_about_z(int32_t p_bone, float angle);
	// void set_rotation(Rot newRotation) {
	// 	this.rotation.set(newRotation);
	// 	this.refreshPrecomputed();
	// }
	int32_t get_parent(int32_t p_bone) const;
	void set_parent(int32_t p_bone, int32_t p_parent);
	void set_shadow_bone_pose_local(int p_bone, const IKBasis &p_value);
	void align_shadow_bone_globals_to(int p_bone, IKNode3D p_target);
	//Transform get_shadow_pose_local(int p_bone) const;
	IKNode3D get_shadow_pose_global(int p_bone) const;
	IKNode3D get_shadow_constraint_axes_global(int p_bone) const;
	Transform get_shadow_constraint_axes_local(int p_bone) const;
	void set_shadow_constraint_axes_local(int p_bone, const IKBasis &value);
	void align_shadow_constraint_globals_to(int p_bone, Transform p_target);
	void set_bone_dirty(int p_bone, bool p_dirty);
	bool get_bone_dirty(int p_bone) const;
	float get_stiffness(int32_t p_bone) const;
	void set_stiffness(int32_t p_bone, float p_stiffness_scalar);
	float get_height(int32_t p_bone) const;
	void set_height(int32_t p_bone, float p_height);

	Ref<KusudamaConstraint> get_constraint(int32_t p_bone) const;
	void set_constraint(int32_t p_bone, Ref<KusudamaConstraint> p_constraint);
	void init(Ref<SkeletonModification3DEWBIK> p_mod);
	int32_t get_bone_count() const;
	void set_bone_count(int32_t p_bone_count);

protected:
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _get(const StringName &p_name, Variant &r_ret) const;
	bool _set(const StringName &p_name, const Variant &p_value);
};
