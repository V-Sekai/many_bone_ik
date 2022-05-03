/*************************************************************************/
/*  ewbik_shadow_bone_3d.h                                               */
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

#ifndef EWBIK_SHADOW_BONE_3D_H
#define EWBIK_SHADOW_BONE_3D_H

#include "core/object/ref_counted.h"
#include "ik_pin_3d.h"
#include "math/ik_transform.h"
#include "scene/3d/skeleton_3d.h"

#define IK_DEFAULT_DAMPENING 0.20944f

class IKManipulator3D;

class IKBone3D : public Resource {
	GDCLASS(IKBone3D, Resource);

private:
	BoneId bone_id = -1;
	Ref<IKBone3D> parent = nullptr;
	Vector<Ref<IKBone3D>> children;
	Ref<IKManipulator3D> pin = nullptr;
	IKTransform3D xform;
	Basis rot_delta;
	Vector3 translation_delta;
	float default_dampening = Math_PI;
	float dampening = get_parent().is_null() ? Math_PI : default_dampening;
	float cos_half_dampen = Math::cos(dampening / 2.0f);
	static bool has_pin_descendant(BoneId p_bone, Skeleton3D *p_skeleton, const HashMap<BoneId, Ref<IKBone3D>> &p_map);

protected:
	static void _bind_methods();

public:
	Vector<BoneId> get_children_with_pin_descendants(Skeleton3D *p_skeleton, const HashMap<BoneId, Ref<IKBone3D>> &p_map) const;
	void set_bone_id(BoneId p_bone_id, Skeleton3D *p_skeleton = nullptr);
	BoneId get_bone_id() const;
	void set_parent(const Ref<IKBone3D> &p_parent);
	Ref<IKBone3D> get_parent() const;
	void set_pin(const Ref<IKManipulator3D> &p_pin);
	Ref<IKManipulator3D> get_pin() const;
	void set_pose(const Transform3D &p_transform);
	Transform3D get_pose() const;
	void set_global_pose(const Transform3D &p_transform);
	void set_global_pose_rot_delta(const Basis &p_rot);
	void set_global_pose_translation_delta(Vector3 p_translation);
	Vector3 get_translation_delta() const;

	Transform3D get_global_pose() const;
	void set_initial_pose(Skeleton3D *p_skeleton);
	void set_skeleton_bone_pose(Skeleton3D *p_skeleton, real_t p_strenght);
	void create_pin();
	bool is_pin() const;
	IKBone3D() {}
	IKBone3D(String p_bone, Skeleton3D *p_skeleton, const Ref<IKBone3D> &p_parent = nullptr, float p_default_dampening = IK_DEFAULT_DAMPENING);
	~IKBone3D() {}
	float get_cos_half_dampen() const;
	void set_cos_half_dampen(float p_cos_half_dampen);
};

#endif // EWBIK_SHADOW_BONE_3D_H