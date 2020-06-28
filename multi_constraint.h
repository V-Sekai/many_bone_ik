/*************************************************************************/
/*  skeleton_ik_constraints.h                                            */
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

#ifndef MULTI_CONSTRAINT_H
#define MULTI_CONSTRAINT_H

#include "bone_effector.h"
#include "core/os/memory.h"
#include "core/reference.h"
#include "kusudama_constraint.h"
#include "scene/resources/skeleton_modification_3d.h"

class Skeleton3D;
class KusudamaConstraint;

class MultiConstraint : public SkeletonModification3D {
	GDCLASS(MultiConstraint, SkeletonModification3D);

	Vector<Ref<BoneEffector>> multi_effector;
	Vector<Ref<KusudamaConstraint>> multi_constraint;
	int32_t constraint_count = 0;
	int32_t effector_count = 0;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();

public:
	virtual void execute(float delta) {}
	virtual void setup_modification(SkeletonModificationStack3D *p_stack) {}

	void add_effector(String p_name, NodePath p_node = NodePath(), Transform p_transform = Transform(), real_t p_budget = 4.0f);
	void register_constraint(Skeleton3D *p_skeleton);
	void set_constraint_count(int32_t p_value);
	int32_t get_constraint_count() const;
	Vector<Ref<BoneEffector>> get_bone_effectors() const;
	int32_t find_effector(String p_name);
	void remove_effector(int32_t p_index);
	int32_t find_constraint(String p_name);
	void set_effector_count(int32_t p_value);
	int32_t get_effector_count() const;
	Ref<BoneEffector> get_effector(int32_t p_index) const;
	void set_effector(int32_t p_index, Ref<BoneEffector> p_effector);
	void set_constraint(int32_t p_index, Ref<KusudamaConstraint> p_constraint);
	Ref<KusudamaConstraint> get_constraint(int32_t p_index) const;
	MultiConstraint();
	~MultiConstraint();
};

#endif //MULTI_CONSTRAINT_H
