@tool
extends Object

class_name ConstraintState

var id: String = ""
var for_bone_id: String = ""
var swing_orientation_transform_id: String = ""
var twist_orientation_transform_id: String = ""
var direct_reference: IKKusudama
var index: int
var swing_transform_idx: int = -1
var twist_transform_idx: int = -1
var painfulness: float = 0.0

var constraint_map: Dictionary = {} # Dictionary[String, ConstraintState]
var constraints_list: Array[ConstraintState] = []
var transforms: Array[TransformState]
var transform_map: Dictionary = {} # Dictionary[String, TransformState]
var assume_valid: bool = false
var bone_map: Dictionary = {} # Dictionary[String, BoneState]

func _init(id: String, for_bone_id: String, swing_orientation_transform_id: String, painfulness: float, twist_orientation_transform_id: String, direct_reference: IKKusudama, constraint_map: Dictionary, constraints_list: Array[ConstraintState], transforms: Array[TransformState] , transform_map: Dictionary, bone_map: Dictionary, assume_valid: bool) -> void:
	self.for_bone_id = for_bone_id
	self.swing_orientation_transform_id = swing_orientation_transform_id
	self.twist_orientation_transform_id = twist_orientation_transform_id
	self.direct_reference = direct_reference
	self.painfulness = painfulness
	self.constraint_map = constraint_map
	self.constraints_list = constraints_list
	self.transforms = transforms
	self.transform_map = transform_map
	self.assume_valid = assume_valid
	self.bone_map = bone_map

# Prunes the constraint state.
func prune() -> void:
	if get_twist_transform() != null:
		get_twist_transform().prune()
	get_swing_transform().prune()
	constraint_map.erase(self.id)
	constraints_list[self.index] = null

# Returns the swing transform.
func get_swing_transform() -> TransformState:
	return transforms[self.swing_transform_idx]

# Returns the twist transform.
func get_twist_transform() -> TransformState:
	if self.twist_transform_idx == -1:
		return null
	return transforms[self.twist_transform_idx]

# Sets the index of the constraint state.
func set_index(index: int) -> void:
	self.index = index

# Returns the index of the constraint state.
func get_index() -> int:
	return self.index

func getPainfulness() ->float:
	return self.painfulness

# Optimizes the constraint state.
func optimize() -> void:
	if self.twist_orientation_transform_id != null:
		var twist_transform: TransformState = transform_map.get(self.twist_orientation_transform_id)
		self.twist_transform_idx = twist_transform.get_index()
	var swing_transform: TransformState = transform_map.get(self.swing_orientation_transform_id)
	self.swing_transform_idx = swing_transform.get_index()

# Validates the constraint state.
func validate() -> void:
	if assume_valid:
		return
	var for_bone: BoneState = bone_map.get(self.for_bone_id)
	if for_bone == null:
		push_error("Constraint '" + self.id + "' claims to constrain bone '" + for_bone_id + "', but no such bone has been registered with this SkeletonState")
	if self.swing_orientation_transform_id == null:
		push_error("Constraint with id '" + self.id + "' claims to have no swing transform, but this transform is required. You may provide an identity transform if you wish to indicate that the constraint's swing space is equivalent to the parent bone's default space")
	var constraint_swing: TransformState = transform_map.get(self.swing_orientation_transform_id)
	if constraint_swing == null:
		push_error("Constraint with id '" + self.id + "' claims to have a swingOrientationTransform with id'" + self.swing_orientation_transform_id + "', but no such transform has been registered with this SkeletonState'")
	if self.twist_orientation_transform_id != null:
		var constraint_twist: TransformState = transform_map.get(self.twist_orientation_transform_id)
		if constraint_twist == null:
			push_error("Constraint with id '" + self.id + "' claims to have a twist transform with id'" + self.twist_orientation_transform_id + "', but no such transform has been registered with this SkeletonState'")

# Returns the ID string of the constraint state.
func get_id_string() -> String:
	return self.id

# Returns the direct reference of the constraint state.
func get_direct_reference() -> IKKusudama:
	return self.direct_reference
