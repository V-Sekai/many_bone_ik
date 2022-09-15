/*************************************************************************/
/*  ewbik.h                                                              */
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
	NodePath skeleton_path;
	Ref<IKBoneSegment> segmented_skeleton;
	int32_t constraint_count = 0;
	Vector<StringName> constraint_names;
	int32_t pin_count = 0;
	Vector<Ref<IKEffectorTemplate>> pins;
	Vector<Ref<IKBone3D>> bone_list;
	Vector<Vector2> kusudama_twist;
	Vector<bool> kusudama_flip_handedness;
	Vector<Vector<Vector4>> kusudama_limit_cones;
	Vector<int> kusudama_limit_cone_count;
	float MAX_KUSUDAMA_LIMIT_CONES = 30;
	int32_t max_ik_iterations = 10;
	float default_damp = Math::deg_to_rad(15.0f);
	bool debug_skeleton = true;
	Ref<IKTransform3D> root_transform = memnew(IKTransform3D);
	bool is_dirty = true;
	bool is_enabled = true;
	NodePath skeleton_node_path = NodePath("..");
	void update_shadow_bones_transform();
	void update_skeleton_bones_transform();
	Vector<Ref<IKEffectorTemplate>> get_bone_effectors() const;

protected:
	void _validate_property(PropertyInfo &property) const;
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();
	virtual void skeleton_changed(Skeleton3D *skeleton);
	virtual void execute(real_t delta);
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_READY: {
				set_process_internal(true);
			} break;
			case NOTIFICATION_INTERNAL_PROCESS: {
				if (!is_enabled) {
					return;
				}
				if (is_dirty) {
					skeleton_changed(get_skeleton());
					is_dirty = false;
				}
				execute(get_process_delta_time());
			} break;
		}
	}

public:
	void set_enabled(bool p_enabled) {
		is_enabled = p_enabled;
	}
	bool get_enabled() const {
		return is_enabled;
	}
	void set_skeleton_node_path(NodePath p_skeleton_node_path) {
		is_dirty = true;
		skeleton_node_path = p_skeleton_node_path;
	}
	NodePath get_skeleton_node_path() {
		return skeleton_node_path;
	}
	Skeleton3D *get_skeleton() const {
		Node *node = get_node_or_null(skeleton_node_path);
		return cast_to<Skeleton3D>(node);
	}
	StringName get_root_bone() const;
	void set_root_bone(const StringName &p_root_bone);
	StringName get_tip_bone() const;
	void set_tip_bone(StringName p_bone);
	Ref<IKBoneSegment> get_segmented_skeleton();
	float get_max_ik_iterations() const;
	void set_max_ik_iterations(const float &p_max_ik_iterations);
	float get_time_budget_millisecond() const;
	void set_time_budget_millisecond(const float &p_time_budget);
	void add_pin(const StringName &p_name, const NodePath &p_target_node = NodePath());
	void remove_pin(int32_t p_index);
	void set_debug_skeleton(bool p_debug_skeleton);
	void set_pin_count(int32_t p_value);
	int32_t get_pin_count() const;
	void set_pin_bone(int32_t p_pin_index, const String &p_bone);
	StringName get_pin_bone_name(int32_t p_effector_index) const;
	void set_pin_bone_name(int32_t p_effector_index, StringName p_name) const;
	void set_pin_nodepath(int32_t p_effector_index, NodePath p_node_path);
	NodePath get_pin_nodepath(int32_t p_effector_index) const;
	int32_t find_effector_id(StringName p_bone_name);
	void set_pin_target_nodepath(int32_t p_effector_index, const NodePath &p_target_node);
	void set_pin_weight(int32_t p_pin_index, const real_t &p_weight) {
		ERR_FAIL_INDEX(p_pin_index, pins.size());
		Ref<IKEffectorTemplate> data = pins[p_pin_index];
		if (data.is_null()) {
			data.instantiate();
			pins.write[p_pin_index] = data;
		}
		data->set_weight(p_weight);
		notify_property_list_changed();
		skeleton_changed(get_skeleton());
	}
	real_t get_pin_weight(int32_t p_pin_index) const {
		ERR_FAIL_INDEX_V(p_pin_index, pins.size(), 0.0);
		const Ref<IKEffectorTemplate> data = pins[p_pin_index];
		return data->get_weight();
	}
	void set_pin_direction_priorities(int32_t p_pin_index, const Vector3 &p_priority_direction) {
		ERR_FAIL_INDEX(p_pin_index, pins.size());
		Ref<IKEffectorTemplate> data = pins[p_pin_index];
		if (data.is_null()) {
			data.instantiate();
			pins.write[p_pin_index] = data;
		}
		data->set_direction_priorities(p_priority_direction);
		notify_property_list_changed();
		skeleton_changed(get_skeleton());
	}
	Vector3 get_pin_direction_priorities(int32_t p_pin_index) const {
		ERR_FAIL_INDEX_V(p_pin_index, pins.size(), Vector3(0, 0, 0));
		const Ref<IKEffectorTemplate> data = pins[p_pin_index];
		return data->get_direction_priorities();
	}
	NodePath get_pin_target_nodepath(int32_t p_pin_index);
	void set_pin_depth_falloff(int32_t p_effector_index, const float p_depth_falloff);
	float get_pin_depth_falloff(int32_t p_effector_index) const;
	real_t get_default_damp() const;
	void set_default_damp(float p_default_damp);
	void set_constraint_count(int32_t p_count);
	int32_t get_constraint_count() const;
	StringName get_constraint_name(int32_t p_index) const;
	void set_constraint_name(int32_t p_index, String p_name);
	void set_kusudama_twist(int32_t p_index, Vector2 p_limit);
	Vector2 get_kusudama_twist(int32_t p_index) const;
	void set_kusudama_limit_cone(int32_t p_bone, int32_t p_index,
			Vector3 p_center, float p_radius);
	Vector3 get_kusudama_limit_cone_center(int32_t p_contraint_index, int32_t p_index) const;
	float get_kusudama_limit_cone_radius(int32_t p_contraint_index, int32_t p_index) const;
	void set_kusudama_limit_cone_center(int32_t p_contraint_index, int32_t p_index, Vector3 p_center);
	void set_kusudama_limit_cone_radius(int32_t p_contraint_index, int32_t p_index, float p_radius);
	int32_t get_kusudama_limit_cone_count(int32_t p_contraint_index) const;
	void set_kusudama_limit_cone_count(int32_t p_constraint_index, int32_t p_count);
	bool get_kusudama_flip_handedness(int32_t p_bone) const;
	void set_kusudama_flip_handedness(int32_t p_bone, bool p_flip);
	EWBIK();
	~EWBIK();
};

#endif // SKELETON_MODIFICATION_3D_EWBIK_H
