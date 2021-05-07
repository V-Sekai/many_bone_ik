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

#include "core/object/reference.h"
#include "core/os/memory.h"
#include "ik_bone_chain.h"
#include "scene/resources/skeleton_modification_3d.h"

class SkeletonModification3DEWBIK : public SkeletonModification3D {
	GDCLASS(SkeletonModification3DEWBIK, SkeletonModification3D);

private:
	Skeleton3D *skeleton = nullptr;
	String root_bone;
	BoneId root_bone_index = -1;
	Ref<IKBoneChain> segmented_skeleton;
	int32_t effector_count = 0;
	Vector<Ref<IKBone3D>> multi_effector;
	HashMap<BoneId, Ref<IKBone3D>> effectors_map;
	Vector<Ref<IKBone3D>> bone_list;
	bool is_dirty = true;
	bool calc_done = false;

	// Task
	int32_t ik_iterations = 1;
	int32_t constraint_stabilization_passes = 4;

	void update_segments();
	void update_effectors_map();
	void update_bone_list();
	void generate_default_effectors();
	void update_shadow_bones_transform();
	void update_skeleton_bones_transform(real_t p_blending_delta);
	bool is_calc_done();

protected:
	virtual void _validate_property(PropertyInfo &property) const override;
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();

	Vector<Ref<IKBone3D>> get_bone_effectors() const;

public:
	void set_ik_iterations(int32_t p_iterations);
	int32_t get_ik_iterations() const;
	void set_root_bone(const String &p_root_bone);
	String get_root_bone() const;
	void set_root_bone_index(BoneId p_index);
	BoneId get_root_bone_index() const;
	void set_target_count(int32_t p_value);
	int32_t get_target_count() const;
	void add_target(const String &p_name, const NodePath &p_target_node = NodePath(),
			bool p_use_node_rot = false, const Transform &p_target_xform = Transform());
	int32_t find_target(const String &p_name) const;
	void remove_target(int32_t p_index);
	Ref<IKBone3D> get_target(int32_t p_index) const;
	void set_target(int32_t p_index, const Ref<IKBone3D> &p_effector);
	void set_target_bone_index(int32_t p_effector_index, int32_t p_bone_index);
	BoneId get_target_bone_index(int32_t p_effector_index) const;
	void set_target_bone(int32_t p_effector_index, const String &p_bone);
	String get_target_bone(int32_t p_effector_index) const;
	void set_target_nodepath(int32_t p_index, const NodePath &p_target_node);
	NodePath get_target_nodepath(int32_t p_index) const;
	void set_target_transform(int32_t p_index, const Transform &p_target_transform);
	Transform get_target_transform(int32_t p_index) const;
	void set_target_use_node_rotation(int32_t p_index, bool p_use_node_rot);
	bool get_target_use_node_rotation(int32_t p_index) const;
	void update_skeleton();

	virtual void execute(float delta) override;
	virtual void setup_modification(SkeletonModificationStack3D *p_stack) override;

	void solve(real_t p_blending_delta);
	void iterated_improved_solver();

	SkeletonModification3DEWBIK();
	~SkeletonModification3DEWBIK();
};

#endif // SKELETON_MODIFICATION_3D_EWBIK_H
