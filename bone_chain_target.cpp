/*************************************************************************/
/*  bone_chain_target.cpp                                                */
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

#include "skeleton_modification_3d_ewbik.h"
#include "bone_chain_target.h"

void EWBIKBoneChainTarget::set_target_priorities(float p_x_priority, float p_y_priority, float p_z_priority) {
	bool x_dir = p_x_priority > 0 ? true : false;
	bool y_dir = p_y_priority > 0 ? true : false;
	bool z_dir = p_z_priority > 0 ? true : false;
	mode_code = 0;
	if (x_dir)
		mode_code += XDir;
	if (y_dir)
		mode_code += YDir;
	if (z_dir)
		mode_code += ZDir;

	sub_target_count = 1;
	if ((mode_code & 1) != 0)
		sub_target_count++;
	if ((mode_code & 2) != 0)
		sub_target_count++;
	if ((mode_code & 4) != 0)
		sub_target_count++;

	x_priority = p_x_priority;
	y_priority = p_y_priority;
	z_priority = p_z_priority;
	chain_item->parent_item->rootwardly_update_falloff_cache_from(for_bone());
}

float EWBIKBoneChainTarget::get_depth_falloff() const {
	return depthFalloff;
}

void EWBIKBoneChainTarget::set_depth_falloff(float depth) {
	depthFalloff = depth;
	chain_item->parent_item->rootwardly_update_falloff_cache_from(for_bone());
}

void EWBIKBoneChainTarget::disable() {
	enabled = false;
}

void EWBIKBoneChainTarget::enable() {
	enabled = true;
}

void EWBIKBoneChainTarget::toggle() {
	if (is_enabled()) {
		disable();
	} else {
		enable();
	}
}

 EWBIKBoneChainTarget::EWBIKBoneChainTarget(Ref<EWBIKSegmentedSkeleton3D> p_chain_item, const Ref<EWBIKBoneEffectorTransform> p_end_effector, bool p_enabled) {
	enabled = p_enabled;
	set_target_priorities(x_priority, y_priority, z_priority);
}

 EWBIKBoneChainTarget::EWBIKBoneChainTarget(const Ref<EWBIKBoneChainTarget> p_other_ct) :
		chain_item(p_other_ct->chain_item),
		end_effector(p_other_ct->end_effector) {
}

 EWBIKBoneChainTarget::EWBIKBoneChainTarget(Ref<EWBIKSegmentedSkeleton3D> p_chain_item, const Ref<EWBIKBoneEffectorTransform> p_end_effector) :
		chain_item(p_chain_item),
		end_effector(p_end_effector) {
}

 EWBIKBoneChainTarget::EWBIKBoneChainTarget() :
		chain_item(NULL),
		end_effector(NULL) {
}

bool EWBIKBoneChainTarget::is_enabled() const {
	return enabled;
}

int EWBIKBoneChainTarget::get_subtarget_count() {
	return sub_target_count;
}

uint8_t EWBIKBoneChainTarget::get_mode_code() const {
	return mode_code;
}

float EWBIKBoneChainTarget::get_x_priority() const {
	return x_priority;
}

float EWBIKBoneChainTarget::get_y_priority() const {
	return y_priority;
}

float EWBIKBoneChainTarget::get_z_priority() const {
	return z_priority;
}

void EWBIKBoneChainTarget::align_to_axes(Transform inAxes) {
	//TODO
	//axes.alignGlobalsTo(inAxes);
}

void EWBIKBoneChainTarget::translate(Vector3 location) {
	BoneId bone = chain_item->bone;
	Ref<EWBIKState> state = chain_item->mod->get_state();
	Transform xform = state->get_shadow_pose_local(bone);
	xform.origin *= location;
	state->set_shadow_bone_pose_local(bone, xform);
}

Vector3 EWBIKBoneChainTarget::get_location() {
	Transform xform = chain_item->mod->get_state()->get_shadow_pose_local(chain_item->bone);
	return xform.origin;
}
Ref<EWBIKSegmentedSkeleton3D> EWBIKBoneChainTarget::for_bone() {
	return chain_item;
}

void EWBIKBoneChainTarget::removal_notification() {
	for (int32_t target_i = 0; target_i < child_targets.size(); target_i++) {
		child_targets.write[target_i]->set_parent_target(get_parent_target());
	}
}
