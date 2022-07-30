/*************************************************************************/
/*  ewbik.h                                     */
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
#include "ik_bone_3d.h"
#include "ik_effector_template.h"
#include "math/ik_transform.h"

class IKBoneSegment;
class EWBIK : public Node {
	GDCLASS(EWBIK, Node);
	StringName root_bone;
	StringName tip_bone;
	bool live_preview = true;
	Skeleton3D *skeleton = nullptr;
	NodePath skeleton_path;
	Ref<IKBoneSegment> segmented_skeleton;
	int32_t constraint_count = 0;
	Vector<StringName> constraint_names;
	int32_t pin_count = 0;
	Vector<Ref<IKEffectorTemplate>> pins;
	Vector<Ref<IKBone3D>> bone_list;
	bool is_dirty = true;
	bool debug_skeleton = false;
	HashMap<int32_t, int32_t> kusudama_limit_cone_count;
	Vector<float> kusudama_twist_from;
	Vector<float> kusudama_twist_to;
	Vector<bool> kusudama_flip_handedness;
	HashMap<int32_t, PackedColorArray> kusudama_limit_cones;
	float MAX_KUSUDAMA_LIMIT_CONES = 30;
	float time_budget_millisecond = 0.6f;
	int32_t ik_iterations = 0;
	int32_t max_ik_iterations = 30;
	float default_damp = Math::deg2rad(5.0f);
	Ref<IKTransform3D> root_transform = memnew(IKTransform3D);
	void update_shadow_bones_transform();
	void update_skeleton_bones_transform();
	void update_skeleton();
	Vector<Ref<IKEffectorTemplate>> get_bone_effectors() const;
	void set_dirty();
protected:
	virtual void _validate_property(PropertyInfo &property) const override;
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();

public:
	StringName get_root_bone() const {
		return root_bone;
	}
	void set_root_bone(const StringName &p_root_bone) {
		root_bone = p_root_bone;
		notify_property_list_changed();
		is_dirty = true;
	}
	StringName get_tip_bone() const {
		return tip_bone;
	}
	void set_tip_bone(StringName p_bone) {
		tip_bone = p_bone;
		notify_property_list_changed();
		is_dirty = true;
	}
	void _notification(int p_what);
	NodePath get_skeleton() const {
		return skeleton_path;
	}
	void set_skeleton(NodePath p_skeleton) {
		skeleton_path = p_skeleton;
		is_dirty = true;
	};
	bool get_live_preview() const {
		return live_preview;
	}
	void set_live_preview(bool p_enabled) {
		live_preview = p_enabled;
	}
	Ref<IKBoneSegment> get_segmented_skeleton();
	float get_max_ik_iterations() const;
	void set_max_ik_iterations(const float &p_max_ik_iterations);
	float get_time_budget_millisecond() const;
	void set_time_budget_millisecond(const float &p_time_budget);
	void _execute(real_t p_delta);
	void add_pin(const StringName &p_name, const NodePath &p_target_node = NodePath(), const bool &p_use_node_rotation = true);
	void remove_pin(int32_t p_index);
	bool get_debug_skeleton() const;
	void set_debug_skeleton(bool p_enabled);
	void set_ik_iterations(int32_t p_iterations);
	int32_t get_ik_iterations() const;
	void set_pin_count(int32_t p_value);
	int32_t get_pin_count() const;
	void set_pin_bone(int32_t p_pin_index, const String &p_bone);
	StringName get_pin_bone_name(int32_t p_effector_index) const;
	int32_t find_effector_id(StringName p_bone_name);
	void set_pin_target_nodepath(int32_t p_effector_index, const NodePath &p_target_node);
	NodePath get_pin_target_nodepath(int32_t p_pin_index);
	void set_pin_depth_falloff(int32_t p_effector_index, const float p_depth_falloff);
	float get_pin_depth_falloff(int32_t p_effector_index) const;
	void set_pin_use_node_rotation(int32_t p_index, bool p_use_node_rot);
	bool get_pin_use_node_rotation(int32_t p_index) const;
	real_t get_default_damp() const;
	void set_default_damp(float p_default_damp);
	void set_constraint_count(int32_t p_count);
	int32_t get_constraint_count() const;
	StringName get_constraint_name(int32_t p_index) const;
	void set_constraint_name(int32_t p_index, String p_name);
	void set_kusudama_twist_from(int32_t p_index, float p_from);
	void set_kusudama_twist_to(int32_t p_index, float p_to);
	float get_kusudama_twist_from(int32_t p_index) const;
	float get_kusudama_twist_to(int32_t p_index) const;
	void set_kusudama_limit_cone(int32_t p_bone, int32_t p_index,
			Vector3 p_center, float p_radius);
	Vector3 get_kusudama_limit_cone_center(int32_t p_effector_index, int32_t p_index) const;
	float get_kusudama_limit_cone_radius(int32_t p_effector_index, int32_t p_index) const;
	void set_kusudama_limit_cone_center(int32_t p_effector_index, int32_t p_index, Vector3 p_center);
	void set_kusudama_limit_cone_radius(int32_t p_effector_index, int32_t p_index, float p_radius);
	int32_t get_kusudama_limit_cone_count(int32_t p_effector_index) const;
	void set_kusudama_limit_cone_count(int32_t p_effector_index, int32_t p_count);
	bool get_kusudama_flip_handedness(int32_t p_bone) const;
	void set_kusudama_flip_handedness(int32_t p_bone, bool p_flip);
	EWBIK();
	~EWBIK();
};

#endif // SKELETON_MODIFICATION_3D_EWBIK_H
