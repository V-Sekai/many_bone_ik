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

#include "core/object/reference.h"
#include "ewbik_bone_effector_3d.h"
#include "math/ik_transform.h"
#include "scene/3d/skeleton_3d.h"

class EWBIKBoneEffector3D;

class EWBIKShadowBone3D : public Reference {
	GDCLASS(EWBIKShadowBone3D, Reference);

private:
	BoneId for_bone = -1;
	Ref<EWBIKShadowBone3D> parent = nullptr;
	Vector<Ref<EWBIKShadowBone3D>> children;
	Ref<EWBIKBoneEffector3D> effector = nullptr;
	IKTransform xform;
	bool orientation_lock = false;

#ifdef DEBUG_ENABLED
	bool bone_updated = false;
#endif

	static bool has_effector_descendant(BoneId p_bone, Skeleton3D *p_skeleton, const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map);

protected:
	static void _bind_methods();

public:
	void set_bone_id(BoneId p_bone_id, Skeleton3D *p_skeleton = nullptr);
	BoneId get_bone_id() const;
	void set_parent(const Ref<EWBIKShadowBone3D> &p_parent);
	Ref<EWBIKShadowBone3D> get_parent() const;
	void set_effector(const Ref<EWBIKBoneEffector3D> &p_effector);
	Ref<EWBIKBoneEffector3D> get_effector() const;
	void set_transform(const Transform &p_transform);
	Transform get_transform() const;
	void set_orientation_lock(const bool p_lock);
	bool get_orientation_lock() const;
	void rotate(const Quat &p_rot);
	void rotate_and_translate(const Quat &p_rot, const Vector3 &p_offset);
	void set_global_transform(const Transform &p_transform);
	Transform get_global_transform() const;
	void set_initial_transform(Skeleton3D *p_skeleton);
	void set_skeleton_bone_transform(Skeleton3D *p_skeleton);
	void create_effector();
	bool is_effector() const;
	Vector<BoneId> get_children_with_effector_descendants(Skeleton3D *p_skeleton, const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map) const;

	EWBIKShadowBone3D() {}
	EWBIKShadowBone3D(BoneId p_bone, const Ref<EWBIKShadowBone3D> &p_parent = nullptr);
	EWBIKShadowBone3D(String p_bone, Skeleton3D *p_skeleton, const Ref<EWBIKShadowBone3D> &p_parent = nullptr);
	~EWBIKShadowBone3D() {}
};

#endif // EWBIK_SHADOW_BONE_3D_H