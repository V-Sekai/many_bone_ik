@tool
extends Object

class_name IKPin

var enabled: bool
var axes: IKNode3D
var for_bone: IKBone
var parent_pin: IKPin
var child_pins: Array[IKPin] = []
var pin_weight: float = 1.0
var mode_code: int = 6
var sub_target_count: int = 4
const X_DIR: int = 1
const Y_DIR: int = 2
const Z_DIR: int = 4
var x_priority: float = 1.0
var y_priority: float = 2.0
var z_priority: float = 1.0
var depth_falloff: float = 0.0

func _init(in_axes: IKNode3D, enabled: bool, bone: IKBone):
	self.enabled = enabled
	self.axes = in_axes
	self.for_bone = bone
	set_target_priorities(x_priority, y_priority, z_priority)

func is_enabled() -> bool:
	return enabled

func toggle():
	if self.is_enabled():
		disable()
	else:
		enable()

func enable():
	self.enabled = true

func disable():
	self.enabled = false

# Pins can be ultimate targets, or intermediary targets.
# By default, each pin is treated as an ultimate target, meaning
# any bones which are ancestors to that pin's end-effector
# are not aware of any pins wich are target of bones descending from that end
# effector.
#
# Changing this value makes ancestor bones aware, and also determines how much
# less
# they care with each level down.
#
# Presuming all descendants of this pin have a falloff of 1, then:
# A pin falloff of 0 on this pin means ONLY this pin and its effector are
# reported to ancestor bones.
# A pin falloff of 1 on this pin means ancestor bones care about this pin /
# effector and all of its descendant effectors descendant from thi's pins
# effector as if this pin/effector were a sibling of theirs
# A pin falloff of 0.5 means ancestor bones care about this pin/effector twice
# as much as they care about direct descendants pins/effectors of this pin's
# effctor.
#
# With each level, the pin falloff of a descendant is taken account for each
# level.
# Meaning, if this pin has a falloff of 1, and its descendent has a falloff of
# 0.5
# then this pin will be reported with full weight,
# it descendant will be reported with full weight,
# the descendant of that pin will be reported with half weight.
# the descendant of that one's descendant will be reported with half weight.
#
# @param depth



func set_depth_falloff(depth: float):
	var prev_depth = self.depth_falloff
	self.depth_falloff = depth
	if ((self.depth_falloff == 0 && prev_depth != 0) || (self.depth_falloff != 0 && prev_depth == 0)):
		self.for_bone.parent_armature.regenerate_shadow_skeleton()
	else:
		self.for_bone.parent_armature.update_shadow_skel_rate_info()


func get_depth_falloff() -> float:
	return depth_falloff

# Sets the priority of the orientation bases which effectors reaching for this
# target will and won't align with.
# If all are set to 0, then the target is treated as a simple position target.
# It's usually better to set at least one of these three values to 0, as giving
# a nonzero value to all three is most often redundant.
#
# This values this function sets are only considered by the orientation aware
# solver.
#
# @param xPriority set to a positive value (recommended between 0 and 1) if you
#                  want the bone's x basis to point in the same direction as
#                  this target's x basis (by this library's convention the x
#                  basis corresponds to a limb's twist)
# @param yPriority set to a positive value (recommended between 0 and 1) if you
#                  want the bone's y basis to point in the same direction as
#                  this target's y basis (by this library's convention the y
#                  basis corresponds to a limb's direction)
# @param zPriority set to a positive value (recommended between 0 and 1) if you
#                  want the bone's z basis to point in the same direction as
#                  this target's z basis (by this library's convention the z
#                  basis corresponds to a limb's twist)
func set_target_priorities(x_priority: float, y_priority: float, z_priority: float):
	var x_dir: bool = x_priority > 0
	var y_dir: bool = y_priority > 0
	var z_dir: bool = z_priority > 0
	mode_code = 0

	if x_dir:
		mode_code += X_DIR
	if y_dir:
		mode_code += Y_DIR
	if z_dir:
		mode_code += Z_DIR

	sub_target_count = 1
	if (mode_code & 1) != 0:
		sub_target_count += 1
	if (mode_code & 2) != 0:
		sub_target_count += 1
	if (mode_code & 4) != 0:
		sub_target_count += 1

	self.x_priority = x_priority
	self.y_priority = y_priority
	self.z_priority = z_priority
	self.for_bone.parent_armature.update_shadow_skel_rate_info()

# @return the number of bases an effector to this target will attempt to align
#         on.
func get_subtarget_count() -> int:
	return sub_target_count

func get_mode_code() -> int:
	return mode_code

# @return the priority of this pin's x axis;
func get_x_priority() -> float:
	return self.x_priority

# @return the priority of this pin's y axis;
func get_y_priority() -> float:
	return self.y_priority

# @return the priority of this pin's z axis;
func get_z_priority() -> float:
	return self.z_priority

func get_axes() -> IKNode3D:
	return axes

# translates the pin to the location specified in global coordinates
#
# @param location
func translate_to_(location: Vector3):
	self.axes.translate_to(location)

func get_for_bone() -> IKBone:
	return self.for_bone

# called when this pin is being removed entirely from the Armature. (as opposed
# to just being disabled)
func removal_notification():
	for cp in child_pins:
		cp.set_parent_pin(get_parent_pin())

func set_parent_pin(parent: IKPin):
	if self.parent_pin != null:
		self.parent_pin.remove_child_pin(self)
	# set the parent to the global axes if the user
	# tries to set the pin to be its own parent

	if parent == self or parent == null:
		self.axes.set_parent(null)
	elif parent != null:
		self.axes.set_parent(parent.axes)
		parent.add_child_pin(self)
		self.parent_pin = parent

func solve_ik_for_this_and_children():
	for child_pin in child_pins:
		child_pin.solve_ik_for_this_and_children()
	self.for_bone.solve_ik_from_here()

func remove_child_pin(child: IKPin):
	child_pins.erase(child)

func add_child_pin(new_child: IKPin):
	if new_child.is_ancestor_of(self):
		self.set_parent_pin(new_child.get_parent_pin())
	if not child_pins.has(new_child):
		child_pins.append(new_child)

func get_parent_pin() -> IKPin:
	return self.parent_pin

func is_ancestor_of(potential_descendent: IKPin) -> bool:
	var result: bool = false
	var cursor: IKPin = potential_descendent.get_parent_pin()
	while cursor != null:
		if cursor == self:
			result = true
			break
		else:
			cursor = cursor.parent_pin
	return result

func get_pin_weight() -> float:
	return pin_weight
	
# Currently only works with tranquil solver.
#
# @param weight any positive number representing how much the IK solver
#               should prefer to satisfy this pin over competing pins. For
#               example, setting
#               one pin's weight to 90 and a competing pins weight to 10 will
#               mean the IK solver
#               will prefer to satisfy the pin with a weight of 90 by as much
#               as 9 times over satisfying
#               the pin with a weight of 10.
#
func set_pin_weight(weight: float) -> void:
	pin_weight = weight
	for_bone.parent_armature.update_shadow_skel_rate_info()
