@tool
extends Object

class_name BoneState

var id: String
var parent_id: String
var transform_id: String
var target_id: String
var constraint_id: String
var stiffness: float = 0.0
var child_map: Dictionary = {} # Dictionary[String, int]
var index: int
var parent_idx: int = -1
var child_indices: Array[int] = []
var transform_idx: int = -1
var constraint_idx: int = -1
var target_idx: int = -1

func _init(_id: String, _transform_id: String, _parent_id: String, _target_id: String, _constraint_id: String, _stiffness: float) -> void:
	id = _id
	parent_id = _parent_id
	transform_id = _transform_id
	target_id = _target_id
	constraint_id = _constraint_id
	stiffness = _stiffness

func get_transform() -> TransformState:
	return transforms[transform_idx]

func get_target() -> TargetState:
	if target_idx == -1:
		return null
	else:
		return targets[target_idx]

func get_stiffness() -> float:
	return stiffness

func get_parent() -> BoneState:
	if parent_idx >= 0:
		return bones[parent_idx]
	return null

func get_child_from_name(_id: String) -> BoneState:
	return bones[child_map[_id]]

func get_child_from_index(index: int) -> BoneState:
	return bones[child_indices[index]]

func clear_child_list() -> void:
	child_map.clear()
	child_indices = []

func get_child_count() -> int:
	return len(child_indices)

func get_temp_child_count() -> int:
	return len(child_map.values())

func get_constraint() -> ConstraintState:
	if constraint_idx >= 0:
		return constraints[constraint_idx]
	return null

func prune() -> void:
	bones_list[index] = null
	bone_map.erase(id)
	get_transform().prune()
	if get_constraint() != null:
		get_constraint().prune()
	if parent_id != null:
		bone_map[parent_id].child_map.erase(id)
	if root_bone_state == self:
		root_bone_state = null

func set_index(_index: int) -> void:
	index = _index
	if parent_id != null:
		var parent_bone: BoneState = bone_map[parent_id]
		parent_bone.add_child(id, index)

func add_child(_id: String, child_index: int) -> void:
	child_map[_id] = child_index

func optimize() -> void:
	var temp_children: Array[int] = child_map.values()
	child_indices = temp_children.duplicate()
	if parent_id != null:
		parent_idx = bone_map[parent_id].index
	transform_idx = transform_map[transform_id].get_index()
	if constraint_id != null:
		constraint_idx = constraint_map[constraint_id].get_index()
	if target_id != null:
		target_idx = target_map[target_id].get_index()

func validate() -> void:
	if assumeValid:
		return

	var transform: TransformState = transformMap.get(self.transform_id)
	if transform == null:
		raise("Bone '%s' references transform with id '%s', but '%s' has not been registered with the SkeletonState." % [self.id, self.transform_id, self.transform_id])

	if self.parent_id != null:
		var parent: BoneState = boneMap.get(self.parent_id)
		if parent == null:
			raise("Bone '%s' references parent bone with id '%s', but '%s' has not been registered with the SkeletonState." % [self.id, self.parent_id, self.parent_id])

		var parentBonesTransform: TransformState = transformMap.get(parent.transform_id)
		var transformsParent: TransformState = transformMap.get(transform.parent_id)

		if parentBonesTransform != transformsParent:
			raise("Bone '%s' has listed bone with id '%s' as its parent, which has a transform_id of '%s' but the parent transform of this bone's transform is listed as %s. A bone's transform must have the parent bone's transform as its parent" % [self.id, self.parent_id, parent.transform_id, transform.parent_id])

		var ancestor: BoneState = parent
		while ancestor != null:
			if ancestor == self:
				raise("Bone '%s' is listed as being both a descendant and an ancestor of itself." % self.id)

			if ancestor.parent_id == null:
				ancestor = null
			else:
				var curr: BoneState = ancestor
				ancestor = boneMap.get(ancestor.parent_id)
				if ancestor == null:
					raise("bone with id `%s` lists its parent bone as having id `%s`, but no such bone has been registered with the SkeletonState" % [curr.id, curr.parent_id])

	else:
		if self.constraint_id != null:
			raise("Bone '%s' has been determined to be a root bone. However, root bones may not be constrained. If you wish to constrain the root bone anyway, please insert a fake unconstrained root bone prior to this bone. Give that bone's transform values equal to this bone's, and set this bone's transforms to identity." % self.id)

	if self.constraint_id != null:
		var constraint: ConstraintState = constraintMap.get(self.constraint_id)
		if constraint == null:
			raise("Bone '%s' claims to be constrained by '%s', but no such constraint has been registered with this SkeletonState" % [self.id, self.constraint_id])

		if not constraint.forBone_id == self.id:
			raise("Bone '%s' claims to be constrained by '%s', but constraint of id '%s' claims to be constraining bone with id '%s'" % [self.id, constraint.id, constraint.id, constraint.forBone_id])

func get_index() -> int:
	return index


func get_id_string() -> String:
	return self.id


func set_stiffness(stiffness: float) -> void:
	self.stiffness = stiffness
