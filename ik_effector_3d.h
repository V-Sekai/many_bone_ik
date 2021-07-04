/*************************************************************************/
/*  ik_effector_3d.h                                             */
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
#ifndef ik_effector_3d_H
#define ik_effector_3d_H

#include "core/object/ref_counted.h"
#include "ik_bone_3d.h"
#include "scene/3d/skeleton_3d.h"

#define MIN_SCALE 0.1

class IKBone3D;

class IKEffector3D : public RefCounted {
	GDCLASS(IKEffector3D, RefCounted);
	friend class IKBone3D;
	friend class IKBoneChain;

private:
	Ref<IKBone3D> for_bone;
	Transform3D target_transform;
	NodePath target_nodepath = NodePath();
	real_t depth_falloff = 0.0;
	Transform3D goal_transform;
	int32_t num_headings;
	Vector3 priority = Vector3(0.5, 5.0, 0.0);
	real_t weight = 1.0;
	bool follow_x, follow_y, follow_z;
	PackedVector3Array target_headings;
	PackedVector3Array tip_headings;
	Vector<real_t> heading_weights;

	Transform3D prev_node_xform;

	void update_priorities();
	void update_goal_transform(Skeleton3D *p_skeleton);

protected:
	static void _bind_methods();
	void create_headings(const Vector<real_t> &p_weights);

public:
	void set_target_transform(const Transform3D &p_target_transform);
	Transform3D get_target_transform() const;
	void set_target_node(const NodePath &p_target_node_path);
	NodePath get_target_node() const;
	Transform3D get_goal_transform() const;
	bool is_node_xform_changed(Skeleton3D *p_skeleton) const;
	Ref<IKBone3D> get_shadow_bone() const;
	void create_weights(Vector<real_t> &p_weights, real_t p_falloff) const;
	bool is_following_translation_only() const;
	void update_effector_target_headings(PackedVector3Array *p_headings, int32_t &p_index, Vector<real_t> *p_weights) const;
	void update_effector_tip_headings(Ref<IKBone3D> p_for_bone, PackedVector3Array *p_headings, int32_t &p_index) const;

	IKEffector3D(const Ref<IKBone3D> &p_for_bone);
	IKEffector3D() {}
	~IKEffector3D() {}
	bool get_follow_x() const { return follow_x; }
	bool get_follow_y() const { return follow_y; }
	bool get_follow_z() const { return follow_z; }
};

#endif // ik_effector_3d_H