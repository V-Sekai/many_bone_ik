@tool
extends Object

class_name EWBIK

var dirty_skel_state: bool = false
var dirty_rate: bool = false
var skel_state: SkeletonState

# A list of the bones used by the solver, in the same order they appear in the skelState after validation.
# This is to very quickly update the scene with the solver's results, without incurring hashmap lookup penalty.
var skel_state_bone_list: Array[AbstractBone] = []

var shadow_skel: ShadowSkeleton

func _regenerate_shadow_skeleton() -> void:
    skel_state = SkeletonState.new()
    for b in bones:
        register_bone_with_shadow_skeleton(b)
    skel_state.validate()
    shadow_skel = ShadowSkeleton.new(skel_state, self)
    skel_state_bone_list = []
    for i in range(bones.size()):
        var bone_state: BoneState = skel_state.get_bone_state_by_id(bones[i].identity_hash)
        if bone_state != null:
            skel_state_bone_list.append(bones[i])
    dirty_skel_state = false

# This method should be called whenever a structural change has been made to the armature prior to calling the solver.
# A structural change is basically any change other than a rotation/translation/scale of a bone or a target.
# Structural changes include things like,
#       1. reparenting / adding / removing bones.
#   2. marking a bone as an effector (aka "pinning / unpinning a bone")
#   3. adding / removing a constraint on a bone.
#   4. modifying a pin's fallOff to non-zero if it was zero, or zero if it was non-zero
#
# You should NOT call this function if you have only modified a translation/rotation/scale of some transform on the armature
#
# For skeletal modifications that are likely to effect the solver behavior but do not fall
# under any of the above (generally things like changing bone stiffness, depth falloff, targetweight, etc) to intermediary values,
# you should (but don't have to) call update_shadow_skel_rate_info() for maximum efficiency.
func regenerate_shadow_skeleton(force: bool = false) -> void:
    dirty_skel_state = true
    if force:
        _regenerate_shadow_skeleton()
    dirty_rate = true

func update_shadow_skel_rate_info() -> void:
    dirty_rate = true

func _update_shadow_skel_rate_info() -> void:
    var bone_states: Array[BoneState] = skel_state.get_bones_array()
    for i in range(skel_state_bone_list.size()):
        var b: AbstractBone = skel_state_bone_list[i]
        var bs: BoneState = bone_states[i]
        bs.set_stiffness(b.get_stiffness())

func register_bone_with_shadow_skeleton(bone: AbstractBone) -> void:
    var par_bone_id: String = bone.parent == null ? null : bone.parent.identity_hash
    var constraint: Constraint = bone.get_constraint()
    var constraint_id: String = constraint == null ? null : constraint.identity_hash
    var target: AbstractIKPin = bone.get_ik_pin()
    var target_id: String = target == null || target.pin_weight == 0 || !target.is_enabled() ? null : target.identity_hash
    skel_state.add_bone(
        bone.identity_hash,
        bone.local_axes().identity_hash,
        par_bone_id,
        constraint_id,
        bone.get_stiffness(),
        target_id)
    register_axes_with_shadow_skeleton(bone.local_axes(), bone.parent == null)
    if target_id != null:
        register_target_with_shadow_skeleton(target)
    if constraint_id != null:
        register_constraint_with_shadow_skeleton(constraint)

func register_target_with_shadow_skeleton(ik_pin: AbstractIKPin) -> void:
    skel_state.add_target(ik_pin.identity_hash,
        ik_pin.axes.identity_hash,
        ik_pin.for_bone().identity_hash,
        [ik_pin.x_priority, ik_pin.y_priority, ik_pin.z_priority],
        ik_pin.depth_falloff,
        ik_pin.pin_weight)
    register_axes_with_shadow_skeleton(ik_pin.axes, true)

func register_constraint_with_shadow_skeleton(constraint: Constraint) -> void:
    var twist_axes: AbstractAxes = constraint.twist_orientation_axes() == null ? null : constraint.twist_orientation_axes()
    skel_state.add_constraint(
        constraint.identity_hash,
        constraint.attached_to().identity_hash,
        constraint.swing_orientation_axes().identity_hash,
        twist_axes == null ? null : twist_axes.identity_hash,
        constraint)
    register_axes_with_shadow_skeleton(constraint.swing_orientation_axes(), false)
    if twist_axes != null:
        register_axes_with_shadow_skeleton(twist_axes, false)

# @param axes
# @param rebase if true, this function will not provide a parent_id for these axes.
# This is mostly useful for ensuring that targetAxes are always implicitly defined in skeleton space when calling the solver.
# You should always set this to true when giving the axes of an IKPin, as well as when giving the axes of the root bone.
# see the skelState.add_transform documentation for more info.
func register_axes_with_shadow_skeleton(axes: AbstractAxes, unparent: bool) -> void:
    var parent_id: String = unparent || axes.parent_axes == null ? null : axes.parent_axes.identity_hash
    var basis: AbstractBasis = get_skel_state_relative_basis(axes, unparent)
    var translate: Vector3 = basis.translate
    var rotation: Quat = basis.rotation
    skel_state.add_transform(
        axes.identity_hash,
        [translate.x, translate.y, translate.z],
        rotation.to_array(),
        [1.0, 1.0, 1.0],
        parent_id, axes)

# @param axes
# @param unparent if true, will return a COPY of the basis in Armature space, otherwise, will return a reference to axes.local_m_basis
# @return
func get_skel_state_relative_basis(axes: AbstractAxes, unparent: bool) -> AbstractBasis:
    var basis: AbstractBasis = axes.get_local_m_basis()
    if unparent:
        basis = basis.copy()
        local_axes().get_global_m_basis().set_to_local_of(axes.get_global_m_basis(), basis)
    return basis
