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

class IKPin3D : public Resource {
	GDCLASS(IKPin3D, Resource);
	friend class IKBone3D;
	friend class IKBoneChain;

private:
	Ref<IKBone3D> for_bone;
	bool use_target_node_rotation = true;
	NodePath target_node;
	ObjectID target_node_cache;
	Node *target_node_reference = nullptr;

	Transform3D goal_transform;
	int32_t num_headings = 0;
	// See IKEffectorTemplate
	// These are set to off values for debugging.
	real_t depth_falloff = 0.0;
	Vector3 priority = Vector3(0.0f, 0.0f, 0.0f);
	////
	real_t weight = 1.0;
	bool follow_x = true, follow_y = true, follow_z= true;
	PackedVector3Array target_headings;
	PackedVector3Array tip_headings;
	Vector<real_t> heading_weights;

	void update_priorities();
	void update_goal_transform(Skeleton3D *p_skeleton);

protected:
	static void _bind_methods();
	void create_headings(const Vector<real_t> &p_weights);

public:
	const float MAX_KUSUDAMA_LIMIT_CONES = 30;
	float get_depth_falloff() const;
	void set_depth_falloff(float p_depth_falloff);
	Vector3 get_priority() const;
	void set_priority(Vector3 p_priority);
	void update_target_cache(Node *p_skeleton);
	void set_target_node(Node *p_skeleton, const NodePath &p_target_node_path);
	NodePath get_target_node() const;
	Transform3D get_goal_transform() const;
	void set_use_target_node_rotation(bool p_use);
	bool get_use_target_node_rotation() const;
	Ref<IKBone3D> get_shadow_bone() const;
	void create_weights(Vector<real_t> &p_weights, real_t p_falloff) const;
	bool is_following_translation_only() const;
	void update_effector_target_headings(PackedVector3Array *p_headings, int32_t &p_index, Vector<real_t> *p_weights) const;
	void update_effector_tip_headings(Ref<IKBone3D> p_current_bone, PackedVector3Array *p_headings, int32_t &p_index) const;
	IKPin3D(const Ref<IKBone3D> &p_current_bone);
	IKPin3D() {}
	~IKPin3D() {}
	bool get_follow_x() const { return follow_x; }
	bool get_follow_y() const { return follow_y; }
	bool get_follow_z() const { return follow_z; }
};

#endif // ik_effector_3d_H