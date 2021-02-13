#include "ewbik_state.h"

#include "ewbik_transform.h"

#include "shadow_bone_3d.h"

#include "core/math/quat.h"

float EWBIKState::get_stiffness(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);
	return bones[p_bone].sim_local_ik_node.get_stiffness();
}

void EWBIKState::set_stiffness(int32_t p_bone, float p_stiffness_scalar) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.set_stiffness(p_stiffness_scalar);
}

float EWBIKState::get_height(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);
	return bones[p_bone].sim_local_ik_node.get_height();
}

void EWBIKState::set_height(int32_t p_bone, float p_height) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	return bones.write[p_bone].sim_local_ik_node.set_height(p_height);
}

Ref<KusudamaConstraint> EWBIKState::get_constraint(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), nullptr);
	return bones[p_bone].get_constraint();
}

void EWBIKState::set_constraint(int32_t p_bone, Ref<KusudamaConstraint> p_constraint) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].set_constraint(p_constraint);
}

void EWBIKState::init(Ref<SkeletonModification3DEWBIK> p_mod) {
	ERR_FAIL_COND(p_mod.is_null());
	mod = p_mod;
	Ref<SkeletonModificationStack3D> stack = p_mod->get_modification_stack();
	ERR_FAIL_COND(stack.is_null());
	Skeleton3D *skeleton = stack->get_skeleton();
	if (!skeleton) {
		return;
	}
	set_bone_count(skeleton->get_bone_count());
	for (int32_t bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
		set_stiffness(bone_i, -1);
		set_height(bone_i, -1);
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		constraint->set_name(skeleton->get_bone_name(bone_i));
		set_constraint(bone_i, constraint);
		Transform xform = skeleton->get_bone_global_pose(bone_i);
		BoneId parent = skeleton->get_bone_parent(bone_i);
		if (parent != -1) {
			xform = skeleton->get_bone_global_pose(parent).affine_inverse() * xform;
		}
		IKBasis basis = IKBasis(xform.origin, xform.basis.get_axis(0), xform.basis.get_axis(1), xform.basis.get_axis(2));
		set_shadow_bone_pose_local(bone_i, basis);
		set_parent(bone_i, skeleton->get_bone_parent(bone_i));
	}
}

int32_t EWBIKState::get_bone_count() const {
	return bone_count;
}

void EWBIKState::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "bone_count"));
	for (int bone_i = 0; bone_i < get_bone_count(); bone_i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, "bone/" + itos(bone_i) + "/name"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/stiffness"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/height"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/twist_min_angle"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/twist_range"));
		p_list->push_back(PropertyInfo(Variant::INT, "bone/" + itos(bone_i) + "/direction_count", PROPERTY_HINT_RANGE, "0,65535,1"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, "bone/" + itos(bone_i) + "/constraint_axes"));
		Ref<KusudamaConstraint> kusudama = get_constraint(bone_i);
		if (kusudama.is_null()) {
			continue;
		}
		for (int j = 0; j < kusudama->get_direction_count(); j++) {
			p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/direction" + "/" + itos(j) + "/radius", PROPERTY_HINT_RANGE, "0,65535,or_greater"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "bone/" + itos(bone_i) + "/direction" + "/" + itos(j) + "/control_point"));
		}
	}
}

bool EWBIKState::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "bone_count") {
		r_ret = get_bone_count();
		return true;
	} else if (name.begins_with("bone/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		if (what == "stiffness") {
			r_ret = get_stiffness(index);
			return true;
		} else if (what == "height") {
			r_ret = get_height(index);
			return true;
		}
		Ref<KusudamaConstraint> kusudama = get_constraint(index);
		if (kusudama.is_null()) {
			return false;
		}
		if (what == "name") {
			r_ret = kusudama->get_name();
			return true;
		} else if (what == "twist_min_angle") {
			r_ret = kusudama->get_twist_constraint()->get_min_twist_angle();
			return true;
		} else if (what == "twist_range") {
			r_ret = kusudama->get_twist_constraint()->get_range();
			return true;
		} else if (what == "limiting") {
			r_ret = kusudama->get_twist_constraint()->get_range();
			return true;
		} else if (what == "direction_count") {
			r_ret = kusudama->get_direction_count();
			return true;
		} else if (what == "constraint_axes") {
			r_ret = kusudama->get_constraint_axes();
			return true;
		} else if (what == "direction") {
			int direction_index = name.get_slicec('/', 3).to_int();
			ERR_FAIL_INDEX_V(direction_index, kusudama->get_direction_count(), false);
			Ref<DirectionConstraint> direction = kusudama->get_direction(direction_index);
			if (direction.is_null()) {
				return false;
			}
			String direction_what = name.get_slicec('/', 4);
			if (direction_what == "radius") {
				r_ret = direction->get_radius();
				return true;
			} else if (direction_what == "control_point") {
				r_ret = direction->get_control_point();
				return true;
			}
		}
	}
	return false;
}

bool EWBIKState::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name == "bone_count") {
		set_bone_count(p_value);
		return true;
	} else if (name.begins_with("bone/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, bone_count, false);
		if (what == "stiffness") {
			set_stiffness(index, p_value);
			_change_notify();
		} else if (what == "height") {
			set_height(index, p_value);
			_change_notify();
		}
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		set_constraint(index, constraint);
		if (what == "name") {
			constraint->set_name(p_value);
			_change_notify();
			return true;
		} else if (what == "twist_min_angle") {
			Ref<TwistConstraint> twist = constraint->get_twist_constraint();
			twist->set_min_twist_angle(p_value);
			constraint->set_twist_constraint(twist);
			_change_notify();
			return true;
		} else if (what == "twist_range") {
			Ref<TwistConstraint> twist = constraint->get_twist_constraint();
			twist->set_range(p_value);
			constraint->set_twist_constraint(twist);
			_change_notify();
			return true;
		} else if (what == "constraint_axes") {
			constraint->set_constraint_axes(p_value);
			_change_notify();
			return true;
		} else if (what == "direction_count") {
			constraint->set_direction_count(p_value);
			_change_notify();
			return true;
		} else if (what == "direction") {
			int direction_index = name.get_slicec('/', 3).to_int();
			ERR_FAIL_INDEX_V(direction_index, constraint->get_direction_count(), false);
			Ref<DirectionConstraint> direction = constraint->get_direction(direction_index);
			if (direction.is_null()) {
				direction.instance();
				constraint->set_direction(direction_index, direction);
			}
			String direction_what = name.get_slicec('/', 4);
			if (direction_what == "radius") {
				direction->set_radius(p_value);
				_change_notify();
			} else if (direction_what == "control_point") {
				direction->set_control_point(p_value);
				// TODO
				// constraint->optimize_limiting_axes();
				_change_notify();
			} else {
				return false;
			}
			return true;
		}
		return true;
	}
	return false;
}
void EWBIKState::set_bone_count(int32_t p_bone_count) {
	bone_count = p_bone_count;
	bones.resize(p_bone_count);
	for (int32_t bone_i = 0; bone_i < p_bone_count; bone_i++) {
		bones.write[bone_i] = ShadowBone3D();
	}
}

IKNode3D EWBIKState::get_shadow_pose_global(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), IKNode3D());
	return bones[p_bone].sim_local_ik_node;
}
//Transform EWBIKState::get_shadow_pose_local(int p_bone) const {
//	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
//	Transform xform;
//	const IKBasis &basis = bones[p_bone].sim_local_ik_node.get_local();
//	xform.origin = basis.get_origin();
//	xform.basis = basis.get_rotation();
//	return xform;
//}
void EWBIKState::set_shadow_bone_pose_local(int p_bone, const IKBasis &p_value) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.set_local(p_value);
	mark_dirty(p_bone);
}
IKNode3D EWBIKState::get_shadow_constraint_axes_global(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), IKNode3D());
	return bones[p_bone].sim_constraint_ik_node;
}
void EWBIKState::mark_dirty(int32_t p_bone) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.dirty = true;
	bones.write[p_bone].sim_constraint_ik_node.dirty = true;
	update_skeleton();
}
bool EWBIKState::is_dirty(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), true);
	return bones[p_bone].sim_local_ik_node.dirty || bones[p_bone].sim_constraint_ik_node.dirty;
}
void EWBIKState::_update_process_order() {
	ShadowBone3D *bonesptr = bones.ptrw();
	int len = bones.size();

	parentless_bones.clear();

	for (int bone_i = 0; bone_i < len; bone_i++) {
		if (bonesptr[bone_i].sim_local_ik_node.parent >= len) {
			//validate this just in case
			ERR_PRINT("Bone " + itos(bone_i) + " has invalid parent: " + itos(bonesptr[bone_i].sim_local_ik_node.parent));
			bonesptr[bone_i].sim_local_ik_node.parent = -1;
		}
		bonesptr[bone_i].sim_local_ik_node.child_bones.clear();

		if (bonesptr[bone_i].sim_local_ik_node.parent != -1) {
			int parent_bone_idx = bonesptr[bone_i].sim_local_ik_node.parent;

			// Check to see if this node is already added to the parent:
			if (bonesptr[parent_bone_idx].sim_local_ik_node.child_bones.find(bone_i) < 0) {
				// Add the child node
				bonesptr[parent_bone_idx].sim_local_ik_node.child_bones.push_back(bone_i);
			} else {
				ERR_PRINT("IkNode3D parenthood graph is cyclic");
			}
		} else {
			parentless_bones.push_back(bone_i);
		}
	}
}
void EWBIKState::translate_to(int32_t p_bone, Vector3 p_target) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	mark_dirty(p_bone);
	if (bonesptr[p_bone].sim_local_ik_node.parent != -1) {
		bonesptr[p_bone].sim_local_ik_node.pose_local.translate_to(bonesptr[bonesptr[p_bone].sim_local_ik_node.parent].sim_local_ik_node.get_global().get_local_of(p_target));
		mark_dirty(p_bone);
	} else {
		bonesptr[p_bone].sim_local_ik_node.pose_local.translate_to(p_target);
		mark_dirty(p_bone);
	}
}
Ray EWBIKState::get_ray_x(int32_t p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Ray());
	ShadowBone3D *bonesptr = bones.ptrw();
	return bonesptr[p_bone].sim_local_ik_node.get_global().get_x_ray();
}
Ray EWBIKState::get_ray_y(int32_t p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Ray());
	ShadowBone3D *bonesptr = bones.ptrw();
	return bonesptr[p_bone].sim_local_ik_node.get_global().get_y_ray();
}
Ray EWBIKState::get_ray_z(int32_t p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Ray());
	ShadowBone3D *bonesptr = bones.ptrw();
	return bonesptr[p_bone].sim_local_ik_node.get_global().get_z_ray();
}
void EWBIKState::rotate_about_x(int32_t p_bone, float angle) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	mark_dirty(p_bone);
	Quat xRot = Quat(bonesptr[p_bone].sim_local_ik_node.get_global().get_x_heading(), angle);
	rotate_by(p_bone, xRot);
	mark_dirty(p_bone);
}
void EWBIKState::rotate_about_y(int32_t p_bone, float angle) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	// bonesptr[p_bone].sim_local_ik_node.update_global();
	mark_dirty(p_bone);
	Quat yRot = Quat(bonesptr[p_bone].sim_local_ik_node.get_global().get_y_heading(), angle);
	rotate_by(p_bone, yRot);
	mark_dirty(p_bone);
}
void EWBIKState::rotate_about_z(int32_t p_bone, float angle) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	// bonesptr[p_bone].sim_local_ik_node.update_global();
	mark_dirty(p_bone);
	Quat zRot = Quat(bonesptr[p_bone].sim_local_ik_node.get_global().get_z_heading(), angle);
	rotate_by(p_bone, zRot);
	mark_dirty(p_bone);
}
void EWBIKState::force_update_bone_children_transforms(int p_bone_idx) {
	ERR_FAIL_INDEX(p_bone_idx, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	List<int> bones_to_process;
	bones_to_process.push_back(p_bone_idx);

	while (bones_to_process.size() > 0) {
		int current_bone_idx = bones_to_process[0];
		bones_to_process.erase(current_bone_idx);

		IKNode3D &bone = bonesptr[current_bone_idx].sim_local_ik_node;
		IKBasis pose = bone.get_local();
		if (bone.parent >= 0) {
			bone.pose_global = bonesptr[bone.parent].sim_local_ik_node.get_global() * pose;
		} else {
			bone.pose_global = pose;
		}
		// Add the bone's children to the list of bones to be processed
		int child_bone_size = bone.child_bones.size();
		for (int i = 0; i < child_bone_size; i++) {
			bones_to_process.push_back(bone.child_bones[i]);
		}
	}
	bones_to_process.push_back(p_bone_idx);
	while (bones_to_process.size() > 0) {
		int current_bone_idx = bones_to_process[0];
		bones_to_process.erase(current_bone_idx);

		IKNode3D &constraint = bonesptr[current_bone_idx].sim_constraint_ik_node;
		IKBasis constraint_axes = constraint.get_local();
		if (constraint.parent >= 0) {
			constraint.pose_global = bonesptr[constraint.parent].sim_constraint_ik_node.get_global() * constraint_axes;
		} else {
			constraint.pose_global = constraint_axes;
		}
		// Add the bone's children to the list of bones to be processed
		int child_bone_size = constraint.child_bones.size();
		for (int i = 0; i < child_bone_size; i++) {
			bones_to_process.push_back(constraint.child_bones[i]);
		}
	}
}
void EWBIKState::update_skeleton() {
	int32_t len = bones.size();
	for (int bone_i = 0; bone_i < len; bone_i++) {
		force_update_bone_children_transforms(bone_i);
		bones.write[bone_i].sim_local_ik_node.dirty = false;
		bones.write[bone_i].sim_constraint_ik_node.dirty = false;
	}
	for (int bone_i = 0; bone_i < parentless_bones.size(); bone_i++) {
		force_update_bone_children_transforms(parentless_bones[bone_i]);
		bones.write[parentless_bones[bone_i]].sim_local_ik_node.dirty = false;
		bones.write[parentless_bones[bone_i]].sim_constraint_ik_node.dirty = false;
	}
}
void EWBIKState::set_parent(int32_t p_bone, int32_t p_parent) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ERR_FAIL_COND(p_parent != -1 && (p_parent < 0));

	bones.write[p_bone].sim_local_ik_node.parent = p_parent;
	bones.write[p_bone].sim_constraint_ik_node.parent = p_parent;
	_update_process_order();
	mark_dirty(p_bone);
}
int32_t EWBIKState::get_parent(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);
	return bones[p_bone].sim_local_ik_node.parent;
}
void EWBIKState::align_shadow_bone_globals_to(int p_bone, IKNode3D p_target) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	force_update_bone_children_transforms(p_bone);
	if (get_parent(p_bone) != -1) {
		IKNode3D shadow_pose_global = get_shadow_pose_global(get_parent(p_bone));
		shadow_pose_global = global_shadow_pose_to_local_pose(p_bone, shadow_pose_global);
		set_shadow_bone_pose_local(p_bone, p_target.get_global());
	} else {
		IKNode3D shadow_pose_global = get_shadow_pose_global(p_bone);
		set_shadow_bone_pose_local(p_bone, shadow_pose_global.get_global());
	}
	mark_dirty(p_bone);
}
void EWBIKState::align_shadow_constraint_globals_to(int p_bone, Transform p_target) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	force_update_bone_children_transforms(p_bone);
	if (get_parent(p_bone) != -1) {
		IKNode3D shadow_constraint_pose_global = get_shadow_constraint_axes_global(get_parent(p_bone));
		shadow_constraint_pose_global = global_constraint_pose_to_local_pose(p_bone, shadow_constraint_pose_global);
		set_shadow_constraint_axes_local(p_bone, shadow_constraint_pose_global.get_global());
	} else {
		IKNode3D shadow_pose_global = get_shadow_constraint_axes_global(p_bone);
		set_shadow_constraint_axes_local(p_bone, shadow_pose_global.get_global());
	}
	mark_dirty(p_bone);
}
void EWBIKState::set_shadow_constraint_axes_local(int p_bone, const IKBasis &value) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_constraint_ik_node.set_local(value);
	mark_dirty(p_bone);
}

Transform EWBIKState::get_shadow_constraint_axes_local(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	Transform xform;
	const IKBasis &basis = bones[p_bone].sim_constraint_ik_node.get_local();
	xform.origin = basis.get_origin();
	xform.basis = basis.get_rotation();

	return xform;
}

IKNode3D EWBIKState::global_shadow_pose_to_local_pose(int p_bone_idx, IKNode3D p_global_pose) {
	if (bones[p_bone_idx].sim_local_ik_node.parent >= 0) {
		int parent_bone_idx = bones[p_bone_idx].sim_local_ik_node.parent;
		IKNode3D ik_node = bones[parent_bone_idx].sim_local_ik_node;
		IKBasis basis = ik_node.get_global();
		basis.translate_by(p_global_pose.get_global().get_origin());
		basis.rotate_by(p_global_pose.get_global().get_rotation());
		// Suspicious TODO
		ik_node.set_local(basis);
		return ik_node;
	} else {
		return p_global_pose;
	}
}
//
//void EWBIKState::translate_constraint_axes_by_global(int32_t p_bone, Vector3 p_translate_by) {
//	ERR_FAIL_INDEX(p_bone, bones.size());
//	IKBasis basis = bones.write[p_bone].sim_constraint_ik_node.get_local();
//	if (bones.write[p_bone].sim_constraint_ik_node.parent != -1) {
//		force_update_bone_children_transforms(p_bone);
//		basis.translate_to(bones.write[p_bone].sim_constraint_ik_node.get_global().get_origin() + p_translate_by);
//	} else {
//		basis.translate_by(p_translate_by);
//	}
//	bones.write[p_bone].sim_constraint_ik_node.set_local(basis);
//	mark_dirty(p_bone);
//}
float EWBIKState::get_cos_half_dampen(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), 0.0f);
	return bones[p_bone].cos_half_dampen;
}
void EWBIKState::set_cos_half_dampen(int32_t p_bone, float p_cos_half_dampen) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].cos_half_dampen = p_cos_half_dampen;
}
Vector<float> EWBIKState::get_half_returnful_dampened(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Vector<float>());
	return bones[p_bone].half_returnful_dampened;
}
Vector<float> EWBIKState::get_cos_half_returnful_dampened(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Vector<float>());
	return bones[p_bone].cos_half_returnful_dampened;
}
void EWBIKState::set_cos_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].cos_half_returnful_dampened = p_dampened;
}

IKNode3D EWBIKState::global_constraint_pose_to_local_pose(int p_bone_idx, IKNode3D p_global_pose) {
	if (bones[p_bone_idx].sim_local_ik_node.parent >= 0) {
		int parent_bone_idx = bones[p_bone_idx].sim_constraint_ik_node.parent;
		IKNode3D ik_node = bones[parent_bone_idx].sim_constraint_ik_node;
		IKBasis basis = ik_node.get_global();
		basis.translate_by(p_global_pose.get_global().get_origin());
		basis.rotate_by(p_global_pose.get_global().get_rotation());
		// Suspicious TODO
		ik_node.set_local(basis);
		return ik_node;
	} else {
		return p_global_pose;
	}
}

void EWBIKState::set_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].half_returnful_dampened = p_dampened;
}

void EWBIKState::set_bone_dirty(int p_bone, bool p_dirty) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.dirty = p_dirty;
	bones.write[p_bone].sim_constraint_ik_node.dirty = p_dirty;
}

bool EWBIKState::get_bone_dirty(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), true);
	return bones[p_bone].sim_local_ik_node.dirty || bones[p_bone].sim_constraint_ik_node.dirty;
}

void EWBIKState::rotate_to(int32_t p_bone, Quat p_rot) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.pose_local.rotate_to(p_rot);
	mark_dirty(p_bone);
}

void EWBIKState::rotate_by(int32_t p_bone, Quat p_add_rotation) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	// bonesptr[p_bone].sim_local_ik_node.update_global();
	if (bonesptr[p_bone].sim_local_ik_node.parent != -1) {
		Quat new_rot = bonesptr[bonesptr[p_bone].sim_local_ik_node.parent].sim_local_ik_node.get_global().get_local_of_rotation(p_add_rotation);
		bonesptr[p_bone].sim_local_ik_node.get_local().rotate_by(new_rot);
	} else {
		bonesptr[p_bone].sim_local_ik_node.get_local().rotate_by(p_add_rotation);
	}
	mark_dirty(p_bone);
}
void EWBIKState::set_springy(int32_t p_bone, bool p_springy) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].springy = p_springy;
}

void EWBIKState::set_pain(int p_bone, const float &p_pain) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].pain = p_pain;
}

float EWBIKState::get_pain(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), 0.0f);
	return bones[p_bone].pain;
}

bool EWBIKState::get_springy(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), false);
	return bones[p_bone].springy;
}