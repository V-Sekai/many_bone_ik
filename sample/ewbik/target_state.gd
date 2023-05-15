@tool
extends Object

class_name TargetState

const X_DIR = 0
const Y_DIR = 2
const Z_DIR = 4

var id: String
var transform_id: String
var for_bone_id: String
var mode_code: int
var priorities: Array[float]
var depth_falloff: float
var weight: float
var index: int
var transform_idx: int
var for_bone_idx: int

var bone_map: Dictionary = {} # Dictionary[String, BoneState]
var transform_map: Dictionary = {} # Dictionary[String, TransformState]
var transforms: Array[TransformState]

var x_dir: bool
var y_dir: bool
var z_dir: bool

# Constructor
func _init(id: String, transform_id: String, for_bone_id: String, priorities: Array[float], depth_falloff: float, weight: float, bone_map: Dictionary, transform_map: Dictionary, transforms: Array[TransformState]) -> void:
	self.bone_map = bone_map
	self.transform_map = transform_map
	self.transforms = transforms
	self.init(id, transform_id, for_bone_id, priorities, depth_falloff, weight)

# Initializes the target state.
func init(id: String, transform_id: String, for_bone_id: String, priorities: Array[float], depth_falloff: float, weight: float) -> void:
	self.id = id
	self.for_bone_id = for_bone_id
	self.transform_id = transform_id
	self.mode_code = 0
	self.priorities = priorities
	self.depth_falloff = depth_falloff
	self.weight = weight

	var x_dir: bool = self.priorities[0] > 0
	var y_dir: bool = self.priorities[1] > 0
	var z_dir: bool = self.priorities[2] > 0

	mode_code = 0
	if x_dir:
		mode_code += 1
	if y_dir:
		mode_code += 2
	if z_dir:
		mode_code += 4

# Sets the index of the target state.
func set_index(index: int) -> void:
	self.index = index

# Optimizes the target state.
func optimize() -> void:
	self.for_bone_idx = bone_map.get(self.for_bone_id).get_index()
	self.transform_idx = transform_map.get(self.transform_id).get_index()

# Returns the associated transform state.
func get_transform() -> TransformState:
	return transforms[self.transform_idx]

# Validates the target state.
func validate() -> void:
	var transform: TransformState = transform_map.get(self.transform_id)

	if transform == null:
		push_error("Target with id '" + self.id + "' lists its transform as having id '" + self.transform_id + "', but no such transform has been registered with this StateSkeleton")

	if transform.parent_id != null:
		push_error("Target with id '" + self.id + "' lists its transform as having a parent transform. However, target transforms are not allowed to have a parent, as they must be given in the space of the skeleton transform. Please provide a transform object that has been converted into skeleton space and has no parent.")

# Returns the ID string of the target state.
func get_id_string() -> String:
	return self.id

# Returns the mode code of the target state.
func get_mode_code() -> int:
	return self.mode_code

# Returns the depth falloff of the target state.
func get_depth_falloff() -> float:
	return self.depth_falloff

# Returns the weight of the target state.
func get_weight() -> float:
	return self.weight

# Returns the index of the target state.
func get_index() -> int:
	return self.index

# Returns the priority of the requested direction.
func get_priority(basis_direction: int) -> float:
	return priorities[basis_direction / 2]

# Returns the priority of whichever direction has the largest priority.
func get_max_priority() -> float:
	var max_pin_weight: float = 0

	if (mode_code & IKPin.X_DIR) != 0:
		max_pin_weight = max(max_pin_weight, self.get_priority(IKPin.X_DIR))

	if (mode_code & IKPin.Y_DIR) != 0:
		max_pin_weight = max(max_pin_weight, self.get_priority(IKPin.Y_DIR))

	if (mode_code & IKPin.Z_DIR) != 0:
		max_pin_weight = max(max_pin_weight, self.get_priority(IKPin.Z_DIR))

	return max_pin_weight
