/*************************************************************************/
/*  ik_bone_chain.h                                        */
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

#ifndef ik_bone_chain_H
#define ik_bone_chain_H

#include "core/object/ref_counted.h"
#include "ik_bone_3d.h"
#include "math/qcp.h"
#include "scene/3d/skeleton_3d.h"

class IKBoneChain : public Resource {
	GDCLASS(IKBoneChain, Resource);

private:
	Ref<IKBone3D> root;
	Ref<IKBone3D> tip;
	Vector<Ref<IKBone3D>> bones;
	Vector<Ref<IKBoneChain>> child_chains; // Contains only direct child chains that end with effectors or have child that end with effectors
	Vector<Ref<IKBoneChain>> effector_direct_descendents;
	Ref<IKBoneChain> parent_chain;
	Vector<Ref<IKPin3D>> effector_list;
	PackedVector3Array target_headings;
	PackedVector3Array tip_headings;
	Vector<real_t> heading_weights;
	int32_t idx_eff_i = -1, idx_eff_f = -1;
	Skeleton3D *skeleton = nullptr;
	QCP qcp;

	BoneId find_root_bone_id(BoneId p_bone);
	Ref<IKBoneChain> get_child_segment_containing(const Ref<IKBone3D> &p_bone);
	void create_headings();
	PackedVector3Array update_target_headings(Ref<IKBone3D> p_for_bone, Vector<real_t> *&p_weights);
	PackedVector3Array update_tip_headings(Ref<IKBone3D> p_for_bone);
	real_t set_optimal_rotation(Ref<IKBone3D> p_for_bone,
			PackedVector3Array &r_htip, PackedVector3Array &r_htarget, const Vector<real_t> &p_weights, float p_dampening = -1, bool p_translate = false);
	void qcp_solver(real_t p_damp, bool p_translate = false);
	void update_optimal_rotation(Ref<IKBone3D> p_for_bone, real_t p_damp, bool p_translate);

	// Orientation cos(angle/2) representation
	Quaternion set_quadrance_angle(Quaternion p_quat, real_t p_cos_half_angle) const;
	Quaternion clamp_to_angle(Quaternion p_quat, real_t p_angle) const;
	Quaternion clamp_to_quadrance_angle(Quaternion p_quat, real_t p_cos_half_angle) const;
protected:
	static void _bind_methods();

public:
	Ref<IKBoneChain> get_parent_chain() {
		return parent_chain;
	}
	void segment_solver(real_t p_damp, bool p_translate = false);
	Ref<IKBone3D> get_root() const;
	Ref<IKBone3D> get_tip() const;
	bool is_root_pinned() const;
	bool is_pin() const;
	Vector<Ref<IKBoneChain>> get_child_chains() const;
	void set_bone_list(Vector<Ref<IKBone3D>> &p_list, bool p_recursive = false, bool p_debug_skeleton = false) const;
	void generate_default_segments_from_root();
	void update_pinned_list();

	IKBoneChain() {}
	IKBoneChain(Skeleton3D *p_skeleton, BoneId p_root_bone, const Ref<IKBoneChain> &p_parent = nullptr);
	~IKBoneChain() {}
};

#endif // ik_bone_chain_H
