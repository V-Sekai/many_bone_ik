@tool
extends Object

class_name IKNode3D

enum AxisDirection {
    NORMAL = 0,
    IGNORE = 1,
    FORWARD = 2
}

enum Side {
    RIGHT = 1,
    LEFT = -1
}

enum Axis {
    X = 0,
    Y = 1,
    Z = 2
}

var debug: bool = false

var localMBasis: IKBasis
var globalMBasis: IKBasis
var parent: IKNode3D = null

var slipType: int = 0
var dirty: bool = true

var dependentsSet: WeakRefSet = WeakRefSet.new()
var dependentsMap: Dictionary = {}

var workingVector: Vector3

var areGlobal: bool = true
var tag: String = str(hash(self)) + "-Axes"

func create_temp_vars(type: Vector3) -> void:
	workingVector = type.copy()

func _init(globalBasis: IKBasis, parent: IKNode3D) -> void:
	self.globalMBasis = globalBasis.copy()
	create_temp_vars(globalBasis.get_origin())
	
	if parent != null:
		set_parent(parent)
	else:
		self.areGlobal = true
		self.localMBasis = globalBasis.copy()

	update_global()

func make_default_vec() -> Vector3:
	return Vector3()


func _init(origin: Vector3, inX: Vector3, inY: Vector3, inZ: Vector3, parent: IKNode3D, customBases: bool = false) -> void:
	if not customBases:
		globalMBasis = parent.get_global_m_basis().copy() if parent != null else AffineBasis(origin)
		localMBasis = parent.get_local_m_basis().copy() if parent != null else AffineBasis(origin)
		globalMBasis.set_identity()
		localMBasis.set_identity()

	if parent == null:
		self.areGlobal = true

	self.parent = parent


func get_parent_axes() -> IKNode3D:
	return self.parent


func update_global(force: bool = false) -> void:
	if self.dirty or force:
		if self.areGlobal:
			globalMBasis.adopt_values(self.localMBasis)
		else:
			get_parent_axes().update_global(false)
			get_parent_axes().get_global_m_basis().set_to_global_of(self.localMBasis, self.globalMBasis)

	dirty = false


func debug_call() -> void:
	pass


func origin_() -> Vector3:
	self.update_global()
	var temp_origin := self.get_global_m_basis().get_origin()
	return temp_origin


func set_parent(intended_parent: IKNode3D, requested_by: Object = null) -> void:
	self.update_global()
	var old_parent := self.get_parent_axes()

	for dependent in dependentsSet:
		dependent.parent_change_warning(self, old_parent, intended_parent, requested_by)

	if intended_parent != null and intended_parent != self:
		intended_parent.update_global()
		intended_parent.get_global_m_basis().set_to_local_of(globalMBasis, localMBasis)

		if old_parent != null:
			old_parent.disown(self)

		self.parent = intended_parent
		self.get_parent_axes().register_dependent(self)
		self.areGlobal = false
	else:
		if old_parent != null:
			old_parent.disown(self)

		self.parent = null
		self.areGlobal = true

	self.mark_dirty()
	self.update_global()

	for dependent in dependentsSet:
		dependent.parent_change_completion_notice(self, old_parent, intended_parent, requested_by)


func for_each_dependent(action: Callable) -> void:
	var i := 0
	while i < dependentsSet.size():
		var dr := dependentsSet[i]
		if dr != null:
			action.call(dr)
		else:
			dependentsSet.remove(i)
			continue

		i += 1


func update_global(force: bool = false) -> void:
	if self.dirty or force:
		if self.areGlobal:
			globalMBasis.adopt_values(self.localMBasis)
		else:
			get_parent_axes().update_global(false)
			get_parent_axes().get_global_m_basis().set_to_global_of(self.localMBasis, self.globalMBasis)

	dirty = false

func get_global_chirality() -> int:
	self.update_global()
	return self.get_global_m_basis().chirality

func get_local_chirality() -> int:
	self.update_global()
	return self.get_local_m_basis().chirality

func is_global_axis_flipped(axis: int) -> bool:
	self.update_global()
	return globalMBasis.is_axis_flipped(axis)

func is_local_axis_flipped(axis: int) -> bool:
	return localMBasis.is_axis_flipped(axis)

func set_relative_to_parent(par: IKNode3D) -> void:
	if self.get_parent_axes() != null:
		self.get_parent_axes().disown(self)

	self.parent = par
	self.areGlobal = false
	self.get_parent_axes().register_dependent(self)
	self.mark_dirty()

func needs_update() -> bool:
	return self.dirty

func get_global_of(input_vector: Vector3) -> Vector3:
	var result := input_vector.duplicate()
	set_to_global_of(input_vector, result)
	return result

func set_to_global_of(input: Vector3, output: Vector3 = null) -> Vector3:
	self.update_global()
	get_global_m_basis().set_to_global_of(input, output if output else input)
	return output if output else input

func set_to_global_of_ray(input: IKRay3D, output: IKRay3D) -> void:
	self.update_global()
	self.set_to_global_of(input.p1(), output.p1())
	self.set_to_global_of(input.p2(), output.p2())


func get_global_of(input: IKRay3D) -> IKRay3D:
	return IKRay3D.new(self.get_global_of(input.p1()), self.get_global_of(input.p2()))

func to_identity() -> void:
	self.local_m_basis.set_identity()
	self.mark_dirty()

func get_local_of(input: Vector3) -> Vector3:
	self.update_global()
	return get_global_m_basis().get_local_of(input)

func set_to_local_of(input: Vector3, output: Vector3 = null) -> Vector3:
	self.update_global()
	var result: Vector3 = input.duplicate() if output is null else output
	self.get_global_m_basis().set_to_local_of(input, result)
	return result

func set_to_local_of_ray(input: IKRay3D, output: IKRay3D) -> void:
	self.set_to_local_of(input.p1(), output.p1())
	self.set_to_local_of(input.p2(), output.p2())


func set_to_local_of(input: Basis, output: Basis) -> void:
	update_global()
	global_m_basis.set_to_local_of(input, output)

func get_local_of(in: IKRay3D) -> IKRay3D:
	var result: IKRay3D = in.copy()
	result.p1 = get_local_of(in.p1)
	result.p2 = get_local_of(in.p2)
	return result

func translate_by_local(translate: Vector3) -> void:
	update_global()
	local_m_basis.translate_by(translate)
	mark_dirty()
	
func translate_by_global(translate: Vector3) -> void:
	if get_parent_axes() != null:
		update_global()
		translate_to(translate + origin())
	else:
		local_m_basis.translate_by(translate)
	mark_dirty()

func translate_to(translate: Vector3, slip: bool) -> void:
	update_global()
	if slip:
		var temp_abstract_axes: IKNode3D = get_global_copy()
		temp_abstract_axes.translate_to(translate)
		slip_to(temp_abstract_axes)
	else:
		translate_to(translate)

func translate_to(translate: Vector3) -> void:
	if get_parent_axes() != null:
		update_global()
		local_m_basis.translate_to(get_parent_axes().global_m_basis.get_local_of(translate))
		mark_dirty()
	else:
		update_global()
		local_m_basis.translate_to(translate)
		mark_dirty()

func set_slip_type(type: int) -> void:
	if get_parent_axes() != null:
		if type == SlipType.IGNORE:
			get_parent_axes().dependents_set.erase(self)
		elif type == SlipType.NORMAL or type == SlipType.FORWARD:
			get_parent_axes().register_dependent(self)
	slip_type = type

func get_slip_type() -> int:
	return slip_type

func rotate_about_x(angle: float, orthonormalized: bool) -> void:
	update_global()
	var x_rot: Quat = Quat(global_m_basis.x_heading, angle)
	rotate_by(x_rot)
	mark_dirty()

func rotate_about_y(angle: float, orthonormalized: bool) -> void:
	update_global()
	var y_rot: Quat = Quat(global_m_basis.y_heading, angle)
	rotate_by(y_rot)
	mark_dirty()

func rotate_about_z(angle: float, orthonormalized: bool) -> void:
	update_global()
	var z_rot: Quat = Quat(global_m_basis.z_heading, angle)
	rotate_by(z_rot)
	mark_dirty()

func rotate_by(apply: Quat) -> void:
	update_global()
	if get_parent_axes() != null:
		var new_rot: Quat = get_parent_axes().global_m_basis.get_local_of_rotation(apply)
		local_m_basis.rotate_by(new_rot)
	else:
		local_m_basis.rotate_by(apply)
	mark_dirty()

func rotate_by_local(apply: Quat) -> void:
	update_global()
	if parent != null:
		local_m_basis.rotate_by(apply)
	mark_dirty()

func align_locals_to(target_axes: IKNode3D) -> void:
	local_m_basis.adopt_values(target_axes.local_m_basis)
	mark_dirty()

func align_globals_to(target_axes: IKNode3D) -> void:
	target_axes.update_global()
	update_global()
	if get_parent_axes() != null:
		get_parent_axes().global_m_basis.set_to_local_of(target_axes.global_m_basis, local_m_basis)
	else:
		local_m_basis.adopt_values(target_axes.global_m_basis)
	mark_dirty()
	update_global()

func align_orientation_to(target_axes: IKNode3D) -> void:
	target_axes.update_global()
	update_global()
	if get_parent_axes() != null:
		globalMBasis.rotate_to(target_axes.globalMBasis.rotation)
		get_parent_axes().globalMBasis.set_to_local_of(globalMBasis, localMBasis)
	else:
		localMBasis.rotate_to(target_axes.globalMBasis.rotation)
	mark_dirty()

func set_global_orientation_to(rotation: Quaternion) -> void:
	update_global()
	if get_parent_axes() != null:
		globalMBasis.rotate_to(rotation)
		get_parent_axes().globalMBasis.set_to_local_of(globalMBasis, localMBasis)
	else:
		localMBasis.rotate_to(rotation)
	mark_dirty()

func set_local_orientation_to(rotation: Quaternion) -> void:
	localMBasis.rotate_to(rotation)
	mark_dirty()

func register_dependent(new_dependent: IKNode3D) -> void:
	if is_ancestor_of(new_dependent):
		transfer_to_parent(new_dependent.get_parent_axes())
	if not dependentsSet.has(new_dependent):
		dependentsSet.add(new_dependent)

func is_ancestor_of(potential_descendent: IKNode3D) -> bool:
	var result: bool = false
	var cursor: IKNode3D = potential_descendent.get_parent_axes()
	while cursor != null:
		if cursor == self:
			result = true
			break
		else:
			cursor = cursor.get_parent_axes()
	return result

func transfer_to_parent(new_parent: IKNode3D) -> void:
	emancipate()
	set_parent(new_parent)

func emancipate() -> void:
	if get_parent_axes() != null:
		update_global()
		var old_parent: IKNode3D = get_parent_axes()
		for ad in dependentsSet:
			ad.parent_change_warning(self, get_parent_axes(), null, null)
		localMBasis.adopt_values(globalMBasis)
		get_parent_axes().disown(self)
		parent = DependencyReference[IKNode3D](null)
		are_global = true
		mark_dirty()
		update_global()
		for ad in dependentsSet:
			ad.parent_change_completion_notice(self, old_parent, null, null)

func disown(child: IKNode3D) -> void:
	dependentsSet.erase(child)

func get_global_m_basis() -> IKBasis:
	update_global()
	return globalMBasis

func get_local_m_basis() -> IKBasis:
	return localMBasis


func axis_slip_warning(global_prior_to_slipping: IKNode3D, global_after_slipping: IKNode3D,
						actual_axis: IKNode3D, dont_warn: Array) -> void:
	update_global()
	if slipType == NORMAL:
		if get_parent_axes() != null:
			var global_vals: IKNode3D = relative_to(global_prior_to_slipping)
			global_vals = global_prior_to_slipping.get_local_of(global_vals)
			localMBasis.adopt_values(globalMBasis)
			mark_dirty()
	elif slipType == FORWARD:
		var global_after_vals: IKNode3D = relative_to(global_after_slipping)
		notify_dependents_of_slip(global_after_vals, dont_warn)

func axis_slip_warning(global_prior_to_slipping: IKNode3D, global_after_slipping: IKNode3D,
						actual_axis: IKNode3D) -> void:
	pass

func axis_slip_completion_notice(global_prior_to_slipping: IKNode3D, global_after_slipping: IKNode3D,
									this_axis: IKNode3D) -> void:
	pass

func slip_to(new_axis_global: IKNode3D) -> void:
	update_global()
	var original_global: IKNode3D = get_global_copy()
	notify_dependents_of_slip(new_axis_global)
	var new_vals: IKNode3D = new_axis_global.free_copy()

	if get_parent_axes() != null:
		new_vals = get_parent_axes().get_local_of(new_vals)
	localMBasis.adopt_values(new_vals.globalMBasis)
	dirty = true
	update_global()

	notify_dependents_of_slip_completion(original_global)

func get_weak_ref_to_parent() -> DependencyReference[IKNode3D]:
	return parent

func set_weak_ref_to_parent(parent_ref: DependencyReference[IKNode3D]) -> void:
	parent = parent_ref


func slip_to(new_axis_global: IKNode3D, dont_warn: Array = []) -> void:
	self.update_global()
	var original_global := self.get_global_copy()
	notify_dependents_of_slip(new_axis_global, dont_warn)
	var new_vals := new_axis_global.get_global_copy()

	if self.get_parent_axes() != null:
		new_vals = get_parent_axes().get_local_of(new_axis_global)

	self.align_globals_to(new_axis_global)
	self.mark_dirty()
	self.update_global()

	notify_dependents_of_slip_completion(original_global, dont_warn)

func notify_dependents_of_slip(new_axis_global: IKNode3D, dont_warn: Array) -> void:
	for dependant in dependentsSet:
		if not dont_warn.has(dependant):
			dependant.axis_slip_warning(self.get_global_copy(), new_axis_global, self, dont_warn)
		else:
			print("skipping: ", dependant)

func notify_dependents_of_slip_completion(global_axis_prior_to_slipping: IKNode3D, dont_warn: Array) -> void:
	for dependant in dependentsSet:
		if not dont_warn.has(dependant):
			dependant.axis_slip_completion_notice(global_axis_prior_to_slipping, self.get_global_copy(), self)
		else:
			print("skipping: ", dependant)

func mark_dirty(depth: int = -1) -> void:
	if not self.dirty:
		self.dirty = true
		self.mark_dependents_dirty(depth - 1)

func mark_dependents_dirty(depth: int) -> void:
	if depth >= 0:
		for dependant in dependentsSet:
			dependant.mark_dirty(depth)

func print() -> String:
	self.update_global()
	var global := "Global: " + get_global_m_basis().print()
	var local := "Local: " + get_local_m_basis().print()
	return global + "\n" + local