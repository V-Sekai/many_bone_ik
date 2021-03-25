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
#include "ewbik_shadow_bone_3d.h"
#include "math/qcp.h"
#include "scene/3d/skeleton_3d.h"

class EWBIKSegmentedSkeleton3D : public Reference {
	GDCLASS(EWBIKSegmentedSkeleton3D, Reference);

private:
	Ref<EWBIKShadowBone3D> root;
	Ref<EWBIKShadowBone3D> tip;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> child_chains; // Contains only child chains that end with effectors
	Vector<Ref<EWBIKSegmentedSkeleton3D>> effector_direct_descendents;
	HashMap<BoneId, Ref<EWBIKShadowBone3D>> bones_map;
	Ref<EWBIKSegmentedSkeleton3D> parent_chain;
	PackedVector3Array target_headings;
	PackedVector3Array tip_headings;
	Vector<real_t> heading_weights;
	int32_t idx_eff_i = -1, idx_eff_f = -1;

	Skeleton3D *skeleton = nullptr;
	QCP qcp;

	BoneId find_root_bone_id(BoneId p_bone);
	void generate_skeleton_segments(const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map);
	void update_segmented_skeleton();
	void update_effector_direct_descendents();
	void generate_bones_map();
	Ref<EWBIKSegmentedSkeleton3D> get_child_segment_containing(const Ref<EWBIKShadowBone3D> &p_bone);
	void create_headings(const Vector<Ref<EWBIKBoneEffector3D>> &p_list);
	void update_target_headings(Ref<EWBIKShadowBone3D> p_for_bone, Vector<Ref<EWBIKBoneEffector3D>> &p_effectors);
	void update_tip_headings(Ref<EWBIKShadowBone3D> p_for_bone, Vector<Ref<EWBIKBoneEffector3D>> &p_effectors);
	real_t get_manual_rmsd() const;
	real_t set_optimal_rotation(Ref<EWBIKShadowBone3D> p_for_bone);
	void segment_solver(int32_t p_stabilization_passes, Vector<Ref<EWBIKBoneEffector3D>> &p_effectors);
	void qcp_solver(int32_t p_stabilization_passes, Vector<Ref<EWBIKBoneEffector3D>> &p_effectors);
	void update_optimal_rotation(Ref<EWBIKShadowBone3D> p_for_bone, Vector<Ref<EWBIKBoneEffector3D>> &p_effectors, bool p_translate,
			int32_t p_stabilization_passes);

protected:
	static void _bind_methods();

public:
	Ref<EWBIKShadowBone3D> get_root() const;
	Ref<EWBIKShadowBone3D> get_tip() const;
	bool is_root_pinned() const;
	bool is_tip_effector() const;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> get_child_chains() const;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> get_effector_direct_descendents() const;
	int32_t get_effector_direct_descendents_size() const;
	void get_bone_list(Vector<Ref<EWBIKShadowBone3D>> &p_list) const;
	void generate_default_segments_from_root();
	void update_effector_list(Vector<Ref<EWBIKBoneEffector3D>> &p_list);
	void grouped_segment_solver(int32_t p_stabilization_passes, Vector<Ref<EWBIKBoneEffector3D>> &p_effectors);
	void debug_print_chains(Vector<bool> p_levels = Vector<bool>());

	EWBIKSegmentedSkeleton3D() {}
	EWBIKSegmentedSkeleton3D(Skeleton3D *p_skeleton, BoneId p_root_bone, const Ref<EWBIKSegmentedSkeleton3D> &p_parent = nullptr);
	EWBIKSegmentedSkeleton3D(Skeleton3D *p_skeleton, BoneId p_root_bone,
			const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map, const Ref<EWBIKSegmentedSkeleton3D> &p_parent = nullptr);
	~EWBIKSegmentedSkeleton3D() {}
};

#endif // EWBIK_SEGMENTED_SKELETON_3D_H
