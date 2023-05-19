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
var transforms: Array[TransformState]
var targets: Array[TargetState]
var bones: Array[BoneState]
var constraints: Array[ConstraintState]
var bones_list: Array[BoneState] = []
var bone_map: Dictionary = {} # Dictionary[String, BoneState]
var root_bone_state: BoneState = null
var transform_map: Dictionary = {} # Dictionary[String, TransformState]
var target_map: Dictionary = {} # Dictionary[String, TargetState]
var constraint_map: Dictionary = {} # Dictionary[String, ConstraintState]

func _init(_id: String, _transform_id: String, _parent_id: String, _target_id: String, 
	_constraint_id: String, _stiffness: float, _transforms: Array[TransformState], 
	_targets: Array[TargetState], _bones: Array[BoneState], _constraints: Array[ConstraintState], 
	_bones_list: Array[BoneState], _bone_map: Dictionary, _root_bone_state: BoneState, 
	_transform_map: Dictionary, _target_map: Dictionary, _constraint_map: Dictionary) -> void:
	id = _id
	parent_id = _parent_id
	transform_id = _transform_id
	target_id = _target_id
	constraint_id = _constraint_id
	stiffness = _stiffness
	transforms = _transforms
	targets = _targets
	bones = _bones
	constraints = _constraints
	bones_list = _bones_list
	bone_map = _bone_map
	root_bone_state = _root_bone_state
	transform_map = _transform_map
	target_map = _target_map
	constraint_map = _constraint_map

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
	var transform: TransformState = transform_map.get(self.transform_id)
	if transform == null:
		push_error("Bone '%s' references transform with id '%s', but '%s' has not been registered with the SkeletonState." % [self.id, self.transform_id, self.transform_id])
		return
		
	if self.parent_id != null:
		var parent: BoneState = bone_map.get(self.parent_id)
		if parent == null:
			push_error("Bone '%s' references parent bone with id '%s', but '%s' has not been registered with the SkeletonState." % [self.id, self.parent_id, self.parent_id])
			return
			
		var parentBonesTransform: TransformState = transform_map.get(parent.transform_id)
		var transformsParent: TransformState = transform_map.get(transform.parent_id)

		if parentBonesTransform != transformsParent:
			push_error("Bone '%s' has listed bone with id '%s' as its parent, which has a transform_id of '%s' but the parent transform of this bone's transform is listed as %s. A bone's transform must have the parent bone's transform as its parent" % [self.id, self.parent_id, parent.transform_id, transform.parent_id])
			return
			
		var ancestor: BoneState = parent
		while ancestor != null:
			if ancestor == self:
				push_error("Bone '%s' is listed as being both a descendant and an ancestor of itself." % self.id)
				return
				
			if ancestor.parent_id == null:
				ancestor = null
			else:
				var curr: BoneState = ancestor
				ancestor = bone_map.get(ancestor.parent_id)
				if ancestor == null:
					push_error("bone with id `%s` lists its parent bone as having id `%s`, but no such bone has been registered with the SkeletonState" % [curr.id, curr.parent_id])
					return
	else:
		if self.constraint_id != null:
			push_error("Bone '%s' has been determined to be a root bone. However, root bones may not be constrained. If you wish to constrain the root bone anyway, please insert a fake unconstrained root bone prior to this bone. Give that bone's transform values equal to this bone's, and set this bone's transforms to identity." % self.id)
			return
	if self.constraint_id != null:
		var constraint: ConstraintState = constraint_map.get(self.constraint_id)
		if constraint == null:
			push_error("Bone '%s' claims to be constrained by '%s', but no such constraint has been registered with this SkeletonState" % [self.id, self.constraint_id])
			return
		if not constraint.forBone_id == self.id:
			push_error("Bone '%s' claims to be constrained by '%s', but constraint of id '%s' claims to be constraining bone with id '%s'" % [self.id, constraint.id, constraint.id, constraint.forBone_id])
			return

func get_index() -> int:
	return index


func get_id_string() -> String:
	return self.id


func set_stiffness(stiffness: float) -> void:
	self.stiffness = stiffness
