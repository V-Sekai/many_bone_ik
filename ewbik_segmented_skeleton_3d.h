/*************************************************************************/
/*  ewbik_segmented_skeleton_3d.h                                        */
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

#ifndef EWBIK_SEGMENTED_SKELETON_3D_H
#define EWBIK_SEGMENTED_SKELETON_3D_H

#include "core/object/reference.h"
#include "scene/3d/skeleton_3d.h"
#include "ewbik_shadow_bone_3d.h"

class EWBIKSegmentedSkeleton3D : public Reference {
	GDCLASS(EWBIKSegmentedSkeleton3D, Reference);

private:
	Ref<EWBIKShadowBone3D> root;
	Ref<EWBIKShadowBone3D> tip;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> child_chains;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> effector_descendents;
	int32_t chain_length;
	HashMap<BoneId, Ref<EWBIKShadowBone3D>> bones_map;
	Ref<EWBIKSegmentedSkeleton3D> parent_chain;

	Skeleton3D *skeleton = nullptr;

	BoneId find_root_bone_id(BoneId p_bone);
	void generate_skeleton_segments(const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map);
	void update_effector_descendents();
	void generate_segment_maps();
	void recursively_align_axes_outward_from(Ref<EWBIKShadowBone3D> p_bone);
	Ref<EWBIKSegmentedSkeleton3D> get_child_segment_containing(Ref<EWBIKShadowBone3D> p_bone);

protected:
	static void _bind_methods();

public:
	Ref<EWBIKShadowBone3D> get_root() const;
	Ref<EWBIKShadowBone3D> get_tip() const;
	int32_t get_chain_length() const;
	bool is_root_effector() const;
	bool is_tip_effector() const;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> get_effector_descendents() const;
	int32_t get_effector_descendents_size() const;
	void generate_default_segments_from_root();
	void align_axes_to_bones();

	EWBIKSegmentedSkeleton3D() {}
	EWBIKSegmentedSkeleton3D(Skeleton3D *p_skeleton, BoneId p_root_bone, Ref<EWBIKSegmentedSkeleton3D> p_parent = nullptr);
	EWBIKSegmentedSkeleton3D(Skeleton3D *p_skeleton, BoneId p_root_bone,
			const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map, Ref<EWBIKSegmentedSkeleton3D> p_parent = nullptr);
	~EWBIKSegmentedSkeleton3D() {}
};

#endif // EWBIK_SEGMENTED_SKELETON_3D_H
