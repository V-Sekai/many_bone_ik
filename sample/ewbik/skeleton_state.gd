@tool
extends Object

class_name SkeletonState

## This class is used as an intermediary between the IK solver and whatever
## Armature system you happen to be using.
## Upon initializing the solver, you must provide it with a prebuilt
## SkeletonState object which indicates
## the transformations and parent child relationships between the bones in your
## armature.
##
## The IKSolver will build its internal representation of the skeleton structure
## from this SkeletonState object.
## Once the solver has finished solving, it will update the SkeletonState
## transforms with its solution, so that you
## may read them back into your armature.
##
## You are free to change the SkeletonState transforms before calling the solver
## again, and the solver will
## operate beginning with the new transforms.
##
## IMPORTANT: you should NOT make any structural changes (where a structural
## change is defined as anything other
## than modifications to the transforms of a bone or constraint) to the
## SkeletonState object after registering it with the solver.
## If you make a structural change, you MUST reregister the SkeletonState object
## with the solver.

var bones_list: Array[BoneState] = []
var transforms_list: Array[TransformState] = []
var targets_list: Array[TargetState] = []
var constraints_list: Array[ConstraintState] = []
var bone_map: Dictionary = {} # Dictionary[String, BoneState]
var transform_map: Dictionary = {} # Dictionary[String, TransformState]
var target_map: Dictionary = {} # Dictionary[String, TargetState]
var constraint_map: Dictionary = {} # Dictionary[String, ConstraintState]

var bones: Array[BoneState]
var targets: Array[TargetState]
var constraints: Array[ConstraintState]
var transforms: Array[TransformState]

var root_bone_state: BoneState = null
var assume_valid: bool = false

## Constructor
func _init(assume_valid: bool = false) -> void:
	self.assume_valid = assume_valid

## Returns the array of TransformState objects.
func get_transforms_array() -> Array:
	return self.transforms

## Returns the BoneState object at the given index.
func get_bone_state(index: int) -> BoneState:
	return self.bones[index]

## Returns the BoneState object by its ID.
func get_bone_state_by_id(id: String) -> BoneState:
	return self.bone_map.get(id)

## Returns the number of bones.
func get_bone_count() -> int:
	return self.bones.size()

## Returns the TransformState object at the given index.
func get_transform_state(index: int) -> TransformState:
	return self.transforms[index]

## Returns the number of transforms.
func get_transform_count() -> int:
	return self.transforms.size()

## Returns the array of BoneState objects.
func get_bones_array() -> Array:
	return self.bones

## Returns the ConstraintState object at the given index.
func get_constraint_state(index: int) -> ConstraintState:
	return self.constraints[index]

## Returns the number of constraints.
func get_constraint_count() -> int:
	return self.constraints.size()

## Returns the TargetState object at the given index.
func get_target_state(index: int) -> TargetState:
	return self.targets[index]

## Returns the number of targets.
func get_target_count() -> int:
	return self.targets.size()

## Validates the skeleton state.
func validate() -> void:
	for bs in bone_map.values():
		if bs.parent_id == null:
			if root_bone_state != null:
				push_error("A skeleton may only have 1 root bone, you attempted to initialize bone of id `" + bs.id + "' as an implicit root (no parent bone), when bone with id '" + root_bone_state.id + "' is already determined as being the implicit root")
			root_bone_state = bs

	for bs in bone_map.values():
		bs.validate()
		bones_list.append(bs)

	for ts in transform_map.values():
		ts.validate()
		transforms_list.append(ts)

	for ts in target_map.values():
		ts.validate()
		targets_list.append(ts)

	for cs in constraint_map.values():
		cs.validate()
		constraints_list.append(cs)

	optimize()
	prune()

## Removes any bones/transforms/constraints that the solver would ignore, then reindex and reconnects everything.
func prune() -> void:
	var leaf_count := 1
	while leaf_count > 0:
		var leaf_bones: Array = []
		for bs in bone_map.values():
			if bs.get_temp_child_count() == 0 and bs.target_id == null:
				leaf_bones.append(bs)
		leaf_count = leaf_bones.size()
		for current_leaf in leaf_bones:
			while current_leaf != null and current_leaf.target_id == null:
				if current_leaf.get_temp_child_count() == 0:
					current_leaf.prune()
					current_leaf = current_leaf.get_parent()
				else:
					break
		optimize()

## Adds a bone to the skeleton state.
##
## @param id            A string by which to identify this bone.
## @param transform_id  The ID string of the transform defining this bone's translation, rotation, and scale relative to its parent bone (or relative to the skeleton, if this is the root bone).
## @param parent_id     Null if this bone is a root bone, otherwise, the ID string of this bone's parent (the bone's parent does not need to have been pre-registered with the SkeletonState, but will need to be eventually registered prior to calling SkeletonState.validate()).
## @param target_id     Optional null if this bone is not an effector, otherwise, the string.
## @param constraint_id Optional ID string of the constraint on this bone, or null if no constraint.
## @param stiffness     Aka friction. Optional value from 1-0 indicating how slowly this bone moves. 1 means it never moves, 0 means it moves as much as the dampening parameter allows.
func add_bone(id: String, transform_id: String, parent_id: String, target_id: String, constraint_id: String, stiffness: float = 0.0) -> BoneState:
	var result := BoneState.new(id, transform_id, parent_id, target_id, constraint_id, stiffness)
	bone_map[id] = result
	return result

## Adds a constraint to the solver.
##
## `id`: string by which to identify this constraint
## `forBone_id`: the id string of the bone this constraint is constraining
## `swingOrientationTransform_id`: the id string of the swing orientation transform
## `twistOrientationTransform_id`: OPTIONAL, only relevant for ball and socket-type region constraints (like Kusudamas, splines, limit cones, etc). A transform specifying the orientation of the twist basis to be used in swing twist decomposition when constraining this bone.
## `directReference`: REQUIRED, a reference to the actual Constraint instance so the solver can call its snapToLimitingAxes function.
func add_constraint(id: String, forBone_id: String, swingOrientationTransform_id: String, painfulness: float, twistOrientationTransform_id: String = "", directReference: IKKusudama = null) -> ConstraintState:
	var con := ConstraintState.new(id, forBone_id, swingOrientationTransform_id, painfulness, twistOrientationTransform_id, directReference, constraint_map, constraints_list, transforms, transform_map, bone_map, assume_valid)
	constraint_map[id] = con
	return con

# @param id           a string by which to identify this bone
# @param transform_id the id string of the transform defining this target's
#                     translation, rotation, and scale.
#                     Note, this transform MUST be provided in a space relative
#                     to the skeleton transform.
#                     The skeleton transform is treated as the identity by the
#                     solver. So if your actual target is defined as being in a
#                     space outside of the skeleton,
#                     or is not a direct child of the skeleton transform, you
#                     must convert it into this space whenever updating the
#                     SkeletonState.
#                     The solver never changes anything about the targets, so
#                     you need not worry about reading this value back from the
#                     SkeletonState, or converting its value
#                     back into your desired space.
# @param for_bone_id  the id string of the effector bone this is a target for
# @param priorities   the orientation priorities for this target. For more
#                     information see the set_target_priorities documentation in
#                     IKPin
# @param depth_falloff the depth_fall_off for this target. value 0-1, with 0
#                     indicating that no effectors
#                     downstream of this once are visible to ancestors of this
#                     target's effector, and 1 indicating that all effectors
#                     immediately downstream of this one are solved for with
#                     equal priority to this one by ancestors bones
#                     of this target's effector. For more information, see the
#                     depth_fall_off documentation in IKPin
# @param weight       the influence weight of this target on the IK solution.
#                     A value between 0 and 1, where 0 means no influence and
#                     1 means full influence on the IK solution.
func add_target(id: String, transform_id: String, for_bone_id: String, priorities: Array = [1.0, 1.0, 0.0], depth_falloff: float = 0.0, weight: float = 1.0) -> TargetState:
	var result = TargetState.new(id, transform_id, for_bone_id, priorities, depth_falloff, weight, bone_map, transform_map, transforms)
	target_map[id] = result
	return result

## @param id              a string by which to identify this transform
## @param translation     an array of THREE numbers corresponding to the
##                        translation of this transform in the space of its
##                        parent transform
## @param rotation        an array of FOUR numbers corresponding to the rotation
##                        of this transform in the space of its parent bone's
##                        transform.
##                        The four numbers should be a Hamilton quaternion
##                        representation (not JPL, important!), in the form [W,
##                        X, Y, Z], where W is the scalar quaternion component
## @param scale           an array of THREE numbers corresponding to scale of
##                        the X, Y, and Z components of this transform. The
##                        convention is that a value of [1,1,1] indicates
##                        a right-handed transform, whereas something like [-1,
##                        1, 1] would indicate a left-handed transform (with the
##                        x axis inverted relative to the parent x-axis)
## @param parent_id       a string indicating the parent transform this
##                        transform is defined in the space of, or null defined
##                        relative to the identity transform.
##                        (the identity transform in this is defined as the
##                        parent of the root bone's transform so as to maximize
##                        numerical precision)
## @param directReference optional. To allow for efficiently referencing the
##                        actual transform to which this TransformState
##                        corresponds without invoking a hash lookup,
##                        you can provide a reference to it here which you can
##                        read back
func add_transform(id: String, translation: Array, rotation: Array, scale: Array, parent_id: String, direct_reference = null) -> void:
	var existing_transform: TransformState = transform_map.get(id)

	if existing_transform == null:
		transform_map[id] = TransformState.new(id, translation, rotation, scale, parent_id, direct_reference, transforms_list, transform_map, transforms, assume_valid)
		return

	if existing_transform.direct_reference == direct_reference:
		existing_transform.update(translation, rotation, scale)
	elif existing_transform.translation != translation or existing_transform.rotation != rotation or existing_transform.scale != scale or parent_id != existing_transform.parent_id:
		push_error("Transform with id '%s' already exists and has contents which are not equivalent to the new transform being provided" % id)

## Retrieves the rootmost `BoneState` of this `SkeletonState`.
##
## @return The root `BoneState` of this `SkeletonState`.
func optimize() -> void:
	# Create dense contiguous arrays by filtering out null elements in the corresponding source ArrayList
	bones.clear()
	for bone in bones_list:
		if bone != null:
			bones.append(bone)

	transforms.clear()
	for transform in transforms_list:
		if transform != null:
			transforms.append(transform)

	constraints.clear()
	for constraint in constraints_list:
		if constraint != null:
			constraints.append(constraint)

	targets.clear()
	for target in targets_list:
		if target != null:
			targets.append(target)

	for i in range(bones.size()):
		bones[i].clear_child_list()

	for i in range(bones.size()):
		bones[i].set_index(i)
	for i in range(transforms.size()):
		transforms[i].set_index(i)
	for i in range(targets.size()):
		targets[i].set_index(i)
	for i in range(constraints.size()):
		constraints[i].set_index(i)

	for bs in bones:
		bs.optimize()
	for ts in transforms:
		ts.optimize()
	for ts in targets:
		ts.optimize()
	for cs in constraints:
		cs.optimize()
