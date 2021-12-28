/*************************************************************************/
/*  skeleton_modification_3d_ewbik.h                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef SKELETON_MODIFICATION_3D_EWBIK_H
#define SKELETON_MODIFICATION_3D_EWBIK_H

#include "core/object/ref_counted.h"
#include "core/os/memory.h"
#include "ik_bone_chain.h"
#include "scene/resources/skeleton_modification_3d.h"

class IKEffector3DData : public Resource {
	GDCLASS(IKEffector3DData, Resource);

public:
	NodePath target_node;
	Vector3 priority = Vector3(1.0, 0.1, 1.0);
	float depth_falloff = 1.0f;
	bool use_target_node_rotation = false;
	IKEffector3DData() {
	}
};

class SkeletonModification3DEWBIK : public SkeletonModification3D {
	GDCLASS(SkeletonModification3DEWBIK, SkeletonModification3D);

private:
	Skeleton3D *skeleton = nullptr;
	String root_bone;
	BoneId root_bone_index = -1;
	Ref<IKBoneChain> segmented_skeleton;
	int32_t constraint_count = 0;
	int32_t effector_count = 0;
	Vector<Ref<IKEffector3DData>> multi_effector;
	HashMap<BoneId, Ref<IKBone3D>> effectors_map;
	Vector<Ref<IKBone3D>> bone_list;
	bool is_dirty = true;
	bool debug_skeleton = false;
	PackedInt32Array kusudama_limit_cone_count;
	PackedFloat32Array kusudana_twist;
	Vector<PackedColorArray> kusudama_limit_cones;
	float MAX_KUSUDAMA_LIMIT_CONES = 30;

	// Task
	int32_t ik_iterations = 10;
	float default_damp = 0.01f;

	void update_segments();
	void update_effectors_map();
	void update_bone_list(bool p_debug_skeleton = false);
	void update_shadow_bones_transform();
	void update_skeleton_bones_transform(real_t p_blending_delta);

protected:
	virtual void _validate_property(PropertyInfo &property) const override;
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();

	Vector<Ref<IKEffector3DData>> get_bone_effectors() const;

public:
	bool get_debug_skeleton() const;
	void set_debug_skeleton(bool p_enabled);
	void set_ik_iterations(int32_t p_iterations);
	int32_t get_ik_iterations() const;
	void set_root_bone(const String &p_root_bone);
	String get_root_bone() const;
	void set_root_bone_index(BoneId p_index);
	BoneId get_root_bone_index() const;
	void set_effector_count(int32_t p_value);
	int32_t get_effector_count() const;
	void add_effector(const String &p_name, const NodePath &p_target_node = NodePath(), const bool &p_use_node_rotation = true);
	void remove_effector(int32_t p_index);
	Ref<IKBone3D> find_effector(const String &p_name) const;
	void set_effector_bone(int32_t p_effector_index, const String &p_bone);
	String get_effector_bone(int32_t p_effector_index) const;
	void set_effector_target_nodepath(int32_t p_effector_index, const NodePath &p_target_node);
	NodePath get_effector_target_nodepath(int32_t p_effector_index);
	void set_effector_depth_falloff(int32_t p_effector_index, const float p_depth_falloff);
	float get_effector_depth_falloff(int32_t p_effector_index) const;
	void set_effector_priority(int32_t p_effector_index, Vector3 p_priority);
	Vector3 get_effector_priority(int32_t p_effector_index) const;

	void set_effector_use_node_rotation(int32_t p_index, bool p_use_node_rot);
	bool get_effector_use_node_rotation(int32_t p_index) const;
	void update_skeleton();

	virtual void _execute(real_t p_delta) override;
	virtual void _setup_modification(SkeletonModificationStack3D *p_stack) override;

	void solve(real_t p_blending_delta);
	void iterated_improved_solver(real_t p_damp);

	SkeletonModification3DEWBIK();
	~SkeletonModification3DEWBIK();
	float get_default_damp() const;
	void set_default_damp(float p_default_damp);

	void set_constraint_count(int32_t p_count) {
		kusudana_twist.resize(p_count);
		kusudama_limit_cones.resize(p_count);
		kusudama_limit_cone_count.resize(p_count);
		constraint_count = p_count;
		notify_property_list_changed();
	}
	int32_t get_constraint_count() const {
		return constraint_count;
	}
	void set_kusudama_twist(int32_t p_index, float p_twist) {
		ERR_FAIL_INDEX(p_index, kusudana_twist.size());
		kusudana_twist.write[p_index] = p_twist;
	}

	float get_kusudama_twist(int32_t p_index) const {
		ERR_FAIL_INDEX_V(p_index, kusudana_twist.size(), 0.0f);
		return kusudana_twist[p_index];
	}

	void set_kusudama_limit_cone(int32_t p_bone, int32_t p_index,
			Vector3 p_center, float p_radius) {
		ERR_FAIL_INDEX(p_bone, kusudama_limit_cones.size());
		ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_bone].size());
		Vector3 center = p_center;
		Color cone;
		cone.r = center.x;
		cone.g = center.y;
		cone.b = center.z;
		cone.a = p_radius;
		kusudama_limit_cones.write[p_bone].write[p_index] = cone;
		notify_property_list_changed();
	}

	Vector3 get_kusudama_limit_cone_center(int32_t p_bone, int32_t p_index) const {
		ERR_FAIL_INDEX_V(p_bone, kusudama_limit_cones.size(), Vector3(0.0f, 1.0f, 0.0f));
		ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_bone].size(), Vector3(0.0, 0.0, 0.0));
		Color cone = kusudama_limit_cones[p_bone][p_index];
		Vector3 ret;
		ret.x = cone.r;
		ret.y = cone.g;
		ret.z = cone.b;
		return ret;
	}

	float get_kusudama_limit_cone_radius(int32_t p_bone, int32_t p_index) const {
		ERR_FAIL_INDEX_V(p_bone, kusudama_limit_cones.size(), 0.0f);
		ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_bone].size(), 0.0f);
		return kusudama_limit_cones[p_bone][p_index].a;
	}

	int32_t get_kusudama_limit_cone_count(int32_t p_bone) const {
		ERR_FAIL_INDEX_V(p_bone, kusudama_limit_cones.size(), 0);
		return kusudama_limit_cones[p_bone].size();
	}

	void set_kusudama_limit_cone_count(int32_t p_bone, int32_t p_count) {
		ERR_FAIL_INDEX(p_bone, kusudama_limit_cone_count.size());
		kusudama_limit_cone_count.write[p_bone] = p_count;
		int32_t old_count = kusudama_limit_cones.size();
		kusudama_limit_cones.write[p_bone].resize(p_count);
		for (int32_t cone_i = p_count; cone_i-- > old_count;) {
			set_kusudama_limit_cone(p_bone, cone_i, Vector3(0.f, 0.f, 1.0f), 1.0f);
		}
		notify_property_list_changed();
	}
};

#endif // SKELETON_MODIFICATION_3D_EWBIK_H
