class_name IKBone3D
extends Resource

var bone_id: int = -1
var parent
var children: Array = []
var pin

var default_dampening: float = PI
var dampening: float = parent == null ? PI : default_dampening
var cos_half_dampen: float = cos(dampening / 2.0)
var cos_half_return_damp: float = 0.0
var return_damp: float = 0.0
var cos_half_returnfulness_dampened: Array = []
var half_returnfulness_dampened: Array = []
var stiffness: float = 0.0
var constraint
var constraint_orientation_transform
var constraint_twist_transform
var godot_skeleton_aligned_transform
var bone_direction_transform

func _init(p_bone: String, p_skeleton: Skeleton3D, p_parent: IKBone3D, p_pins: Array, p_default_dampening: float, p_many_bone_ik: ManyBoneIK3D):
    assert(p_skeleton != null)

    default_dampening = p_default_dampening
    cos_half_dampen = cos(default_dampening / 2.0)
    set_name(p_bone)
    bone_id = p_skeleton.find_bone(p_bone)
    if p_parent != null:
        set_parent(p_parent)
    
    for elem in p_pins:
        if elem == null:
            continue
        if elem.get_name() == p_bone:
            create_pin()
            var effector = get_pin()
            effector.set_target_node(p_skeleton, elem.get_target_node())
            effector.set_passthrough_factor(elem.get_passthrough_factor())
            effector.set_weight(elem.get_weight())
            effector.set_direction_priorities(elem.get_direction_priorities())
            break
    
    bone_direction_transform.set_parent(godot_skeleton_aligned_transform)

    var predamp = 1.0 - get_stiffness()
    dampening = p_parent == null ? PI : predamp * p_default_dampening
    var iterations = p_many_bone_ik.get_iterations_per_frame()
    if get_constraint() == null:
        var new_constraint = IKKusudama3D.new()
        add_constraint(new_constraint)
    
    var returnfulness = get_constraint().get_resistance()
    var falloff = 0.2
    half_returnfulness_dampened.resize(iterations)
    cos_half_returnfulness_dampened.resize(iterations)
    var iterations_pow = pow(iterations, falloff * iterations * returnfulness)
    for i in range(iterations):
        var iteration_scalar = ((iterations_pow)-pow(i, falloff * iterations * returnfulness)) / (iterations_pow)
        var iteration_return_clamp = iteration_scalar * returnfulness * dampening
        var cos_iteration_return_clamp = cos(iteration_return_clamp / 2.0)
        half_returnfulness_dampened[i] = iteration_return_clamp
        cos_half_returnfulness_dampened[i] = cos_iteration_return_clamp

func get_cos_half_returnfullness_dampened() -> Array:
    return []

func set_cos_half_returnfullness_dampened(p_value: Array):
    pass

func get_half_returnfullness_dampened() -> Array:
    return []

func set_half_returnfullness_dampened(p_value: Array):
    pass

func set_stiffness(p_stiffness: float):
    pass

func get_stiffness() -> float:
    return 0.0

func is_orientationally_constrained() -> bool:
    var constraint = get_constraint()
    if constraint == null:
        return false
    return constraint.is_orientationally_constrained()

func is_axially_constrained() -> bool:
    var constraint = get_constraint()
    if constraint == null:
        return false
    return constraint.is_axially_constrained()

func get_bone_direction_global_pose() -> Transform:
    return bone_direction_transform.get_global_transform()

func get_bone_direction_transform():
    return null

func set_bone_direction_transform(p_bone_direction):
    pass

func update_default_bone_direction_transform(p_skeleton: Skeleton3D):
    var child_centroid = Vector3()
    var child_count = 0

    for ik_bone in children:
        child_centroid += ik_bone.get_ik_transform().get_global_transform().origin
        child_count += 1

    if child_count > 0:
        child_centroid /= child_count
    else:
        var bone_children = p_skeleton.get_bone_children(bone_id)
        for child_bone_idx in bone_children:
            child_centroid += p_skeleton.get_bone_global_pose(child_bone_idx).origin
        child_centroid /= bone_children.size()

    var godot_bone_origin = godot_skeleton_aligned_transform.get_global_transform().origin
    child_centroid -= godot_bone_origin

    if is_zero_approx(child_centroid.length_squared()):
        if parent.is_valid():
            child_centroid = parent.get_bone_direction_transform().get_global_transform().basis.get_column(Vector3.AXIS_Y)
        else:
            child_centroid = get_bone_direction_transform().get_global_transform().basis.get_column(Vector3.AXIS_Y)

    if not is_zero_approx(child_centroid.length_squared()) and (children.size() or p_skeleton.get_bone_children(bone_id).size()):
        child_centroid.normalize()
        var bone_direction = bone_direction_transform.get_global_transform().basis.get_column(Vector3.AXIS_Y)
        bone_direction.normalize()
        bone_direction_transform.rotate_local_with_global(Quaternion(child_centroid, bone_direction))


func set_constraint_orientation_transform(p_transform):
    pass

func get_constraint_orientation_transform():
    return null

func get_constraint_twist_transform():
    return null

func update_default_constraint_transform():
    var parent_bone = get_parent()
    if parent_bone.is_valid():
        var parent_bone_aligned_transform = get_parent_bone_aligned_transform()
        constraint_orientation_transform.set_global_transform(parent_bone_aligned_transform)

    var set_constraint_twist_transform = get_set_constraint_twist_transform()
    constraint_twist_transform.set_global_transform(set_constraint_twist_transform)

    if constraint.is_null():
        return

    var cones = constraint.get_open_cones()
    var direction
    if cones.size() == 0:
        direction = bone_direction_transform.get_global_transform().basis.get_column(Vector3.AXIS_Y)
    else:
        var total_radius_sum = calculate_total_radius_sum(cones)
        direction = calculate_weighted_direction(cones, total_radius_sum)
        direction -= constraint_orientation_transform.get_global_transform().origin

    var twist_axis = set_constraint_twist_transform.basis.get_column(Vector3.AXIS_Y)
    var align_dir = Quaternion(twist_axis, direction)
    constraint_twist_transform.rotate_local_with_global(align_dir)

func add_constraint(p_constraint):
    pass

func get_constraint():
    return null

func set_bone_id(p_bone_id: int, p_skeleton = null):
    pass

func get_bone_id() -> int:
    return 0

func set_parent(p_parent: Ref<IKBone3D>):
    assert(p_parent != null)
    parent = p_parent
    if parent.is_valid():
        parent.children.append(self)
        godot_skeleton_aligned_transform.set_parent(parent.godot_skeleton_aligned_transform)
        constraint_orientation_transform.set_parent(parent.godot_skeleton_aligned_transform)
        constraint_twist_transform.set_parent(parent.godot_skeleton_aligned_transform)

func get_parent():
    return parent

func set_pin(p_pin: IKEffector3D) -> void:
    pin = p_pin

func get_pin() -> IKEffector3D:
    return pin

func set_global_pose(p_transform: Transform3D):
    godot_skeleton_aligned_transform.set_global_transform(p_transform)
    var transform = constraint_orientation_transform.get_transform()
    transform.origin = godot_skeleton_aligned_transform.get_transform().origin
    constraint_orientation_transform.set_transform(transform)
    constraint_orientation_transform._propagate_transform_changed()

func get_global_pose() -> Transform3D:
    return godot_skeleton_aligned_transform.get_global_transform()

func set_pose(p_transform: Transform3D):
    godot_skeleton_aligned_transform.set_transform(p_transform)

func get_pose() -> Transform3D:
    return godot_skeleton_aligned_transform.get_transform()

func set_initial_pose(p_skeleton: Skeleton) -> void:
    assert(p_skeleton != null, "Skeleton cannot be null.")
    
    if bone_id == -1:
        return
    
    var bone_origin_to_parent_origin: Transform = p_skeleton.get_bone_pose(bone_id)
    self.set_pose(bone_origin_to_parent_origin)

func set_skeleton_bone_pose(p_skeleton: Skeleton3D) -> void:
    assert(p_skeleton != null, "Skeleton3D instance cannot be null")

    var bone_to_parent: Transform = get_pose()

    p_skeleton.set_bone_pose_position(bone_id, bone_to_parent.origin)

    if not bone_to_parent.basis.is_finite():
        bone_to_parent.basis = Basis()

    p_skeleton.set_bone_pose_rotation(bone_id, bone_to_parent.basis.get_rotation_quaternion())
    p_skeleton.set_bone_pose_scale(bone_id, bone_to_parent.basis.get_scale())

func create_pin():
    pin = IKEffector3D.new(self)

func is_pinned() -> bool:
    return pin != null

func get_ik_transform():
    return null

func _init():
    pass

func _exit_tree():
    pass

func get_cos_half_dampen() -> float:
    return 0.0

func set_cos_half_dampen(p_cos_half_dampen: float):
    pass

func get_parent_bone_aligned_transform() -> Transform3D:
    var parent_bone = get_parent()
    if parent_bone == null:
        return Transform3D()
    var parent_bone_aligned_transform = parent_bone.get_ik_transform().get_global_transform()
    parent_bone_aligned_transform.origin = get_bone_direction_transform().get_global_transform().origin
    return parent_bone_aligned_transform

func get_set_constraint_twist_transform() -> Transform3D:
    return constraint_orientation_transform.get_global_transform()

func calculate_total_radius_sum(p_cones: Array) -> float:
    var total_radius_sum = 0.0
    for cone in p_cones:
        if cone == null:
            break
        total_radius_sum += cone.get_radius()
    return total_radius_sum

func calculate_weighted_direction(p_cones, p_total_radius_sum: float) -> Vector3:
    var direction = Vector3()
    for i in range(p_cones.size()):
        var cone = p_cones[i]
        if cone.is_null():
            break
        var weight = cone.get_radius() / p_total_radius_sum
        direction += cone.get_control_point() * weight

    direction.normalize()
    direction = constraint_orientation_transform.get_global_transform().basis.xform(direction)
    return direction
