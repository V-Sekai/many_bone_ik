/*************************************************************************/
/*  bone_chain_target.h                                                  */
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
#ifndef bone_chain_target_h__
#define bone_chain_target_h__

#include "bone_chain_item.h"
#include "core/reference.h"

class DMIKBoneEffectorTransform;
class DMIKShadowSkeletonBone;

class DMIKBoneChainTarget : public Reference {
	GDCLASS(DMIKBoneChainTarget, Reference);

protected:
	bool enabled = true;
	DMIKBoneChainTarget *parent_target = nullptr;
	Vector<DMIKBoneChainTarget *> child_targets;
	float target_weight = 1;
	uint8_t mode_code = 7;
	int sub_target_count = 4;
	float x_priority = 1.0f, y_priority = 1.0f, z_priority = 1.0f;
	float depthFalloff = 0.0f;

public:
	Ref<DMIKShadowSkeletonBone> chain_item = nullptr;
	Ref<DMIKBoneEffectorTransform> end_effector = nullptr;

	DMIKBoneChainTarget();

	DMIKBoneChainTarget(Ref<DMIKShadowSkeletonBone> p_chain_item, const Ref<DMIKBoneEffectorTransform> p_end_effector);

	DMIKBoneChainTarget(const Ref<DMIKBoneChainTarget> p_other_ct);

	DMIKBoneChainTarget(Ref<DMIKShadowSkeletonBone> p_chain_item, const Ref<DMIKBoneEffectorTransform> p_end_effector, bool p_enabled);

public:
	static const short XDir = 1, YDir = 2, ZDir = 4;

	bool is_enabled() const;

	void toggle();

	void enable();

	void disable();

	/**
     * Targets can be ultimate targets, or intermediary targets.
     * By default, each target is treated as an ultimate target, meaning
     * any bones which are ancestors to that target's end-effector
     * are not aware of any target which are target of bones descending from that end effector.
     *
     * Changing this value makes ancestor bones aware, and also determines how much less
     * they care with each level down.
     *
     * Presuming all descendants of this target have a falloff of 1, then:
     * A target falloff of 0 on this target means only this target is reported to ancestors.
     * A target falloff of 1 on this target means ancestors care about all descendant target equally (after accounting for their pinWeight),
     * regardless of how many levels down they are.
     * A target falloff of 0.5 means each descendant target is cared about half as much as its ancestor.
     *
     * With each level, the target falloff of a descendant is taken account for each level.
     *  Meaning, if this target has a falloff of 1, and its descendent has a falloff of 0.5
     *  then this target will be reported with full weight,
     *  it descendant will be reported with full weight,
     *  the descendant of that target will be reported with half weight.
     *  the desecendant of that one's descendant will be reported with quarter weight.
     *
     * @param p_depth
     */
	void set_depth_falloff(float p_depth);

	float get_depth_falloff() const;

	/**
     * Sets the priority of the orientation bases which effectors reaching for this target will and won't align with.
     * If all are set to 0, then the target is treated as a simple position target.
     * It's usually better to set at least on of these three values to 0, as giving a nonzero value to all three is most often redundant.
     *
     *  This values this function sets are only considered by the orientation aware solver.
     *
     * @param position
     * @param p_x_priority set to a positive value (recommended between 0 and 1) if you want the bone's x basis to point in the same direction as this target's x basis (by this library's convention the x basis corresponds to a limb's twist)
     * @param p_y_priority set to a positive value (recommended between 0 and 1)  if you want the bone's y basis to point in the same direction as this target's y basis (by this library's convention the y basis corresponds to a limb's direction)
     * @param p_z_priority set to a positive value (recommended between 0 and 1)  if you want the bone's z basis to point in the same direction as this target's z basis (by this library's convention the z basis corresponds to a limb's twist)
     */
	void set_target_priorities(float p_x_priority, float p_y_priority, float p_z_priority);

	/**
     * @return the number of bases an effector to this target will attempt to align on.
     */
	int get_subtarget_count();

	uint8_t get_mode_code() const;

	/**
     * @return the priority of this target's x axis;
     */
	float get_x_priority() const;

	/**
     * @return the priority of this target's y axis;
     */
	float get_y_priority() const;

	/**
     * @return the priority of this target's z axis;
     */
	float get_z_priority() const;

	Transform get_axes() const;

	/**
     * translates and rotates the target to match the position
     * and orientation of the input Axes. The orientation
     * is only relevant for orientation aware solvers.
     * @param inAxes
     */
	void align_to_axes(Transform inAxes);

	/**
     * translates the pin to the location specified in local coordinates
     * (relative to any other Axes objects the pin may be parented to)
     * @param location
     */
	void translate(Vector3 location);

	/**
     * @return the target location in global coordinates
     */
	Vector3 get_location();

	Ref<DMIKShadowSkeletonBone> for_bone();

	/**
     * called when this target is being removed entirely from the Armature. (as opposed to just being disabled)
     */
	void removal_notification();
	void set_parent_target(DMIKBoneChainTarget *parent);
	void remove_child_target(DMIKBoneChainTarget *child);
	void add_child_target(DMIKBoneChainTarget *new_child);
	DMIKBoneChainTarget *get_parent_target();
	bool is_ancestor_of(DMIKBoneChainTarget *potential_descendent);
	float get_target_weight();
};

#endif // bone_chain_target_h__