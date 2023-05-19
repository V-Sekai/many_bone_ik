@tool
extends Object

class_name IKBone

var parent_armature: Skeleton3D
var tag: String

var last_rotation: Quaternion
var previous_orientation: IKNode3D
var local_axes: IKNode3D
var major_rotation_axes: IKNode3D

var bone_height: float
var parent: IKBone

var children: Array = []
var free_children: Array = []
var effectored_children: Array = []

var constraints: IKKusudama
var pin: IKPin = null
var orientation_lock: bool = false
var stiffness_scalar: float = 0.0
var ancestor_count: int = 0

enum FrameType {
	GLOBAL,
	RELATIVE
}

func _init(par: IKBone = null,
			tip_heading: Vector3 = Vector3(),
			roll_heading: Vector3 = Vector3(),
			input_tag: String = "",
			input_bone_height: float = 0.0,
			coordinate_type: FrameType = FrameType.RELATIVE) -> void:
	last_rotation = Quaternion()

	if par != null:
		tag = str(hash(self)) if input_tag == "" else input_tag
		bone_height = input_bone_height

		var tip_heading_ray: IKRay3D = IKRay3D.new(par.get_tip(), tip_heading)
		var roll_heading_ray: IKRay3D = IKRay3D.new(par.get_tip(), roll_heading)
		var temp_tip: Vector3 = tip_heading
		var temp_roll: Vector3 = roll_heading
		var temp_x: Vector3 = temp_roll

		if coordinate_type == FrameType.GLOBAL:
			temp_tip = tip_heading_ray.heading()
			temp_roll = roll_heading_ray.heading()
		elif coordinate_type == FrameType.RELATIVE:
			temp_tip = par.local_axes.get_global_of(tip_heading_ray.heading())
			temp_roll = par.local_axes.get_global_of(roll_heading_ray.heading())

		temp_x = temp_tip.cross(temp_roll)
		temp_roll = temp_x.cross(temp_tip)

		temp_x = temp_x.normalized()
		temp_tip = temp_tip.normalized()
		temp_roll = temp_roll.normalized()

		self.parent = par
		self.parent_armature = self.parent.parent_armature
		parent_armature.add_to_bone_list(self)

		generate_axes(parent.get_tip(), temp_x, temp_tip, temp_roll)
		local_axes.set_parent(parent.local_axes)

		previous_orientation = local_axes.attached_copy(true)

		major_rotation_axes = parent.local_axes.get_global_copy()
		major_rotation_axes.translate_to(parent.get_tip())
		major_rotation_axes.set_parent(parent.local_axes)

		self.parent.add_free_child(self)
		self.parent.add_child(self)
		self.update_ancestor_count()


func set_ancestor_count(count: int) -> void:
	self.ancestor_count = count
	for b in self.children:
		b.set_ancestor_count(self.ancestor_count + 1)

# /**
# 	* updates the ancestor count for this bone, and
# 	* sets the ancestor count of all child bones
# 	* to this bone's ancestor count +1;
# 	*
# 	* @param count
# 	*/
func update_ancestor_count() -> void:
	var counted_ancestors: int = 0
	var current_bone: IKBone = self.parent
	while current_bone != null:
		counted_ancestors += 1
		current_bone = current_bone.parent
	set_ancestor_count(counted_ancestors)


func get_parent() -> IKBone:
	return self.parent

func attach_to_parent(input_parent: IKBone) -> void:
	input_parent.add_child(self)
	self.parent = input_parent
	self.update_ancestor_count()

func solve_ik_from_here() -> void:
	parent_armature.ik_solver(self)

func set_axes_to_returnfulled(to_set: IKNode3D, swing_axes: IKNode3D, twist_axes: IKNode3D,
								cos_half_angle_dampen: float, angle_dampen: float) -> void:
	if constraints and constraints is IKKusudama:
		constraints.set_axes_to_returnfulled(to_set, swing_axes, twist_axes, cos_half_angle_dampen, angle_dampen)

func set_pin_(pin: Vector3) -> void:
	if not self.pin:
		self.enable_pin_(pin)
	else:
		self.pin.translate_to_(pin)

func add_constraint(new_constraint: IKKusudama) -> IKKusudama:
	constraints = new_constraint
	parent_armature.regenerate_shadow_skeleton()
	return constraints

func get_constraint() -> IKKusudama:
	return constraints

func rotate_by(rot: Quaternion) -> void:
	previous_orientation.align_locals_to(local_axes)
	local_axes.rotate_by(rot)
	last_rotation = rot

func set_frame_of_rotation(rotation_frame_coordinates: IKNode3D) -> void:
	major_rotation_axes.align_locals_to(rotation_frame_coordinates)
	if parent:
		major_rotation_axes.translate_to(parent.get_tip_())

func get_major_rotation_axes() -> IKNode3D:
	return major_rotation_axes

func disable_pin() -> void:
	pin.disable()
	if effectored_children.size() == 0:
		notify_ancestors_of_unpin()
	update_segmented_armature()

func remove_pin() -> void:
	pin.disable()
	if effectored_children.size() == 0:
		notify_ancestors_of_unpin()
	pin.removal_notification()
	update_segmented_armature()

func enable_pin() -> void:
	if not pin:
		pin_axes = get_local_axes().get_global_copy()
		pin_axes.set_parent(parent_armature.get_local_axes().get_parent_axes())
		pin = create_and_return_pin_on_axes(pin_axes)
	pin.enable()
	free_children.clear()
	for child in get_children():
		if child.pin and not child.pin.is_enabled():
			add_free_child(child)

	notify_ancestors_of_pin(false)
	update_segmented_armature()

func enable_pin_(pin_to: Vector3) -> void:
	if not pin:
		pin_axes = get_local_axes().get_global_copy()
		pin_axes.set_parent(parent_armature.get_local_axes().get_parent_axes())
		pin_axes.translate_to(pin_to)
		pin = create_and_return_pin_on_axes(pin_axes)
	else:
		pin.translate_to_(pin_to)
	pin.enable()
	free_children.clear()
	for child in get_children():
		if not child.pin.is_enabled():
			add_free_child(child)
	notify_ancestors_of_pin()
	# Do not update_segmented_armature.

func is_pinned() -> bool:
	return pin and pin.is_enabled()

func toggle_pin() -> void:
	if not self.pin:
		self.enable_pin()
	self.pin.toggle()
	update_segmented_armature()

func return_children_with_pinned_descendants() -> Array[IKBone]:
	children_with_pinned = []
	for c in get_children():
		if c.has_pinned_descendant():
			children_with_pinned.append(c)
	return children_with_pinned

func get_most_immediately_pinned_descendants() -> Array[IKBone]:
	most_immediate_pinned_descendants = []
	add_self_if_pinned(most_immediate_pinned_descendants)
	return most_immediate_pinned_descendants

func get_pinned_axes() -> IKNode3D:
	if not self.pin:
		return null
	return self.pin.get_axes()

func add_self_if_pinned(pinned_bones: Array[IKBone]) -> void:
	if is_pinned():
		pinned_bones.append(self)
	else:
		for child in get_children():
			child.add_self_if_pinned(pinned_bones)

func notify_ancestors_of_pin(update_segments: bool = true) -> void:
	if parent:
		parent.add_to_effectored(self)
	if update_segments:
		parent_armature.regenerate_shadow_skeleton()

func notify_ancestors_of_unpin() -> void:
	if parent:
		parent.remove_from_effectored(self)
	parent_armature.regenerate_shadow_skeleton()

func add_to_effectored(abstract_bone: IKBone) -> void:
	free_index = free_children.find(abstract_bone)
	if free_index != -1:
		free_children.erase(free_index)

	if not effectored_children.has(abstract_bone):
		effectored_children.append(abstract_bone)

	if parent:
		parent.add_to_effectored(self)

func remove_from_effectored(abstract_bone: IKBone) -> void:
	effectored_index = effectored_children.find(abstract_bone)
	if effectored_index != -1:
		effectored_children.erase(effectored_index)

	if not free_children.has(abstract_bone):
		add_free_child(abstract_bone)

	if parent and effectored_children.size() == 0 and pin and pin.is_enabled():
		parent.remove_from_effectored(self)

func get_pinned_root_bone() -> IKBone:
	root_bone = self
	while root_bone.parent and not root_bone.parent.pin.is_enabled():
		root_bone = root_bone.parent
	return root_bone

func update_segmented_armature() -> void:
	parent_armature.regenerate_shadow_skeleton()

func set_tag(new_tag: String) -> void:
	parent_armature.update_bone_tag(self, tag, new_tag)
	tag = new_tag

func get_tag() -> String:
	return tag

func get_base_() -> Vector3:
	return local_axes.origin_().copy()

func get_tip_() -> Vector3:
	return local_axes.y_().scaled(bone_height)

func set_bone_height(in_bone_height: float) -> void:
	bone_height = in_bone_height
	for child in get_children():
		child.get_local_axes().translate_to(get_tip_())
		child.major_rotation_axes.translate_to(get_tip_())

func delete_bone() -> void:
	bones = []
	root = parent_armature.root_bone
	root.has_child(bones, self)
	for p in bones:
		p.remove_from_effectored(self)
		for ab in effectored_children:
			p.remove_from_effectored(ab)
		p.get_children().erase(self)
		p.free_children.erase(self)
	parent_armature.remove_from_bone_list(self)

func has_child(list: Array[IKBone], query: IKBone) -> void:
	if get_children().has(query):
		list.append(self)
	for c in get_children():
		c.has_child(list, query)

func get_local_axes() -> IKNode3D:
	return local_axes

func get_bone_height() -> float:
	return bone_height

func has_pinned_descendant() -> bool:
	if is_pinned():
		return true
	else:
		for c in get_children():
			if c.has_pinned_descendant():
				return true
		return false

func get_ik_pin() -> IKPin:
	return pin

func set_ik_orientation_lock(val: bool) -> void:
	orientation_lock = val
	parent_armature.update_shadow_skel_rate_info()

func get_ik_orientation_lock() -> bool:
	return orientation_lock

func add_child(bone: IKBone) -> void:
	if not get_children().has(bone):
		get_children().append(bone)

func add_free_child(bone: IKBone) -> void:
	if not free_children.has(bone):
		free_children.append(bone)

func add_effectored_child(bone: IKBone) -> void:
	if not effectored_children.has(bone):
		effectored_children.append(bone)
	parent_armature.regenerate_shadow_skeleton()

func add_descendants_to_armature() -> void:
	for b in get_children():
		parent_armature.add_to_bone_list(b)
		b.add_descendants_to_armature()

func get_children() -> Array[IKBone]:
	return children

func set_children(children: Array[IKBone]) -> void:
	self.children = children

func get_stiffness() -> float:
	if get_ik_orientation_lock():
		return 1.0
	return stiffness_scalar

func set_stiffness(stiffness: float) -> void:
	stiffness_scalar = stiffness
	if parent_armature:
		parent_armature.update_shadow_skel_rate_info()

func notify_of_load_completion() -> void:
	set_bone_height(bone_height)
	for b in children:
		b.attach_to_parent(self)
	parent_armature.add_to_bone_list(self)

func compare_to(i: IKBone) -> int:
	return ancestor_count - i.ancestor_count

func print() -> String:
	return get_tag()
