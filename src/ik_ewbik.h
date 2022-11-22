/*************************************************************************/
/*  ik_ewbik.h                                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef IK_EWBIK_H
#define IK_EWBIK_H

#include "core/object/ref_counted.h"
#include "core/os/memory.h"
#include "ik_bone_3d.h"
#include "ik_effector_template.h"
#include "math/ik_node_3d.h"

class IKBoneSegment;
class NBoneIK : public Node3D {
	GDCLASS(NBoneIK, Node3D);
public:
	enum {
		NBONE_IK_EDIT_CONSTRAIN_MODE_OFF = 0,
		NBONE_IK_EDIT_CONSTRAIN_MODE_AUTOMATIC_UNLOCK,
		NBONE_IK_EDIT_CONSTRAIN_MODE_LOCK
	};
private:
	int constrain_mode = NBONE_IK_EDIT_CONSTRAIN_MODE_AUTOMATIC_UNLOCK;
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
	Vector<Vector<Vector4>> kusudama_limit_cones;
	Vector<int> kusudama_limit_cone_count;
	float MAX_KUSUDAMA_LIMIT_CONES = 30;
	int32_t iterations_per_frame = 10;
	float default_damp = Math::deg_to_rad(10.0f);
	bool queue_debug_skeleton = false;
	Ref<IKNode3D> root_transform = Ref<IKNode3D>(memnew(IKNode3D));
	bool is_dirty = true;
	NodePath skeleton_node_path = NodePath("..");
	void update_ik_bones_transform();
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
	void _notification(int p_what);

public:
	void set_skeleton_node_path(NodePath p_skeleton_node_path);
	NodePath get_skeleton_node_path();
	Skeleton3D *get_skeleton() const;

	int get_edit_constraint_mode() const;
	void set_edit_constraint_mode(int p_enable);
	StringName get_root_bone() const;
	void set_root_bone(const StringName &p_root_bone);
	StringName get_tip_bone() const;
	void set_tip_bone(StringName p_bone);
	Ref<IKBoneSegment> get_segmented_skeleton();
	float get_iterations_per_frame() const;
	void set_iterations_per_frame(const float &p_iterations_per_frame);
	void remove_pin(int32_t p_index);
	void queue_print_skeleton();
	void set_pin_count(int32_t p_value);
	int32_t get_pin_count() const;
	void remove_constraint(int32_t p_index);
	void set_pin_bone(int32_t p_pin_index, const String &p_bone);
	StringName get_pin_bone_name(int32_t p_effector_index) const;
	void set_pin_bone_name(int32_t p_effector_index, StringName p_name) const;
	void set_pin_nodepath(int32_t p_effector_index, NodePath p_node_path);
	NodePath get_pin_nodepath(int32_t p_effector_index) const;
	int32_t find_effector_id(StringName p_bone_name);
	void set_pin_target_nodepath(int32_t p_effector_index, const NodePath &p_target_node);
	void set_pin_weight(int32_t p_pin_index, const real_t &p_weight);
	real_t get_pin_weight(int32_t p_pin_index) const;
	void set_pin_direction_priorities(int32_t p_pin_index, const Vector3 &p_priority_direction);
	Vector3 get_pin_direction_priorities(int32_t p_pin_index) const;
	NodePath get_pin_target_nodepath(int32_t p_pin_index);
	void set_pin_passthrough_factor(int32_t p_effector_index, const float p_passthrough_factor);
	float get_pin_passthrough_factor(int32_t p_effector_index) const;
	real_t get_default_damp() const;
	void set_default_damp(float p_default_damp);
	void set_constraint_count(int32_t p_count);
	int32_t find_constraint(String p_string) const;
	int32_t get_constraint_count() const;
	StringName get_constraint_name(int32_t p_index) const;
	void set_constraint_name(int32_t p_index, String p_name);
	void set_kusudama_twist(int32_t p_index, Vector2 p_limit);
	real_t get_kusudama_twist_current(int32_t p_index);
	void set_kusudama_twist_current(int32_t p_index, real_t p_rotation);
	Vector2 get_kusudama_twist(int32_t p_index) const;
	void set_kusudama_limit_cone(int32_t p_bone, int32_t p_index,
			Vector3 p_center, float p_radius);
	Vector3 get_kusudama_limit_cone_center(int32_t p_contraint_index, int32_t p_index) const;
	float get_kusudama_limit_cone_radius(int32_t p_contraint_index, int32_t p_index) const;
	void set_kusudama_limit_cone_center(int32_t p_contraint_index, int32_t p_index, Vector3 p_center);
	void set_kusudama_limit_cone_radius(int32_t p_contraint_index, int32_t p_index, float p_radius);
	int32_t get_kusudama_limit_cone_count(int32_t p_contraint_index) const;
	void set_kusudama_limit_cone_count(int32_t p_constraint_index, int32_t p_count);
	NBoneIK();
	~NBoneIK();
	void set_dirty();
};

#endif // IK_EWBIK_H
