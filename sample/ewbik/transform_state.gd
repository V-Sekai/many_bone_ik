@tool
extends Object

class_name TransformState

var id: String
var translation: Array[float]
var rotation: Array[float]
var scale: Array[float]
var parent_id: String
var direct_reference = null
var index: int
var parent_idx: int = -1
var child_indices: Array[int] = []
var child_idxs_list: Array[int] = []

var transforms_list: Array[TransformState] = []
var transform_map: Dictionary = {} # Dictionary[String, TransformState]
var transforms: Array[TransformState]
var assume_valid: bool = false

# Constructor
func _init(id: String, translation: Array[float], rotation: Array[float], scale: Array[float], parent_id: String, direct_reference, transforms_list: Array[TransformState], transform_map: Dictionary, transforms: Array[TransformState], assume_valid: bool) -> void:
    self.id = id
    self.translation = translation
    self.rotation = rotation
    self.scale = scale
    self.parent_id = parent_id
    self.direct_reference = direct_reference
    self.transforms_list = transforms_list
    self.transform_map = transform_map
    self.transforms = transforms
    self.assume_valid = assume_valid

# Updates the transform state.
func update(translation: Array[float], rotation: Array[float], scale: Array[float]) -> void:
    self.translation = translation
    self.rotation = rotation
    self.scale = scale

# Prunes the transform state.
func prune() -> void:
    transforms_list[self.index] = null
    transform_map.erase(self.id)

# Returns the index of the transform state.
func get_index() -> int:
    return self.index

# Returns the parent index of the transform state.
func get_parent_index() -> int:
    return self.parent_idx

# Returns the parent transform state.
func get_parent_transform() -> TransformState:
    return transforms[self.parent_idx]

# Sets the index of the transform state.
func set_index(index: int) -> void:
    self.index = index
    var par_transform: TransformState = transform_map.get(self.parent_id)
    if par_transform != null:
        par_transform.add_child(self.index)

# Adds a child index to the transform state.
func add_child(child_index: int) -> void:
    self.child_idxs_list.append(child_index)

# Optimizes the transform state.
func optimize() -> void:
    self.child_indices = child_idxs_list
    var ax: TransformState = transform_map.get(self.parent_id)
    if self.parent_id != null:
        self.parent_idx = transform_map.get(self.parent_id).get_index()

# Validates the transform state.
func validate() -> void:
    if assume_valid:
        return
    if self.parent_id == null:
        return

    var parent: TransformState = transform_map.get(parent_id)
    if parent == null:
        push_error("Transform '" + self.id + "' lists '" + self.parent_id + "' as its parent, but no transform with id '" + self.parent_id + "' has been registered with this SkeletonState")
    var ancestor: TransformState = parent
    while ancestor != null:
        if ancestor == self:
            push_error("Transform '" + self.id + "' is listed as being both a descendant and an ancestor of itself.")
        if ancestor.parent_id == null:
            ancestor = null
        else:
            var curr: TransformState = ancestor
            ancestor = transform_map.get(ancestor.parent_id)
            if ancestor == null:
                push_error("Transform with id `" + curr.id + "` lists its parent transform as having id `" + curr.parent_id + "', but no such transform has been registered with the SkeletonState")

# Returns the ID string of the transform state.
func get_id_string() -> String:
    return self.id
