extends Object

class_name WorkingBone

var for_bone: BoneState
var constraint_state: ConstraintState
var constraint: IKKusudama
var target_state: TargetState
var sim_target_axes: IKNode3D
var sim_local_axes: IKNode3D
var sim_constraint_swing_axes: IKNode3D
var sim_constraint_twist_axes: IKNode3D
var on_chain: IKArmatureSegment
var cos_half_dampen: float = 0.0
var cos_half_return_damp: float = 0.0
var return_damp: float = 0.0
var cos_half_returnfullness_dampened_iterated: Array[float]
var half_returnfullness_dampened_iterated: Array[float]
var springy: bool = false

# All of these vectors point in the same direction (the one the bone is pointing in),
# each of them corresponds to a tangent cone which the bone should prefer to avoid.
var bone_tip_vectors: Array[Vector3]

var limit_cone_local_direction_cache: Array[Vector3]
var cosine_radii_cache: Array[float]

func _init(to_simulate: BoneState, chain: IKArmatureSegment, sim_transforms, for_armature) -> void:
	for_bone = to_simulate
	constraint_state = for_bone.get_constraint()
	sim_local_axes = sim_transforms[for_bone.get_transform().get_index()]
	on_chain = chain
	if for_bone.get_target() != null:
		target_state = for_bone.get_target()
		sim_target_axes = sim_transforms[target_state.get_transform().get_index()]

	var pre_damp: float = 1.0 - for_bone.get_stiffness()
	var default_dampening: float = for_armature.get_dampening()
	var dampening: float = PI if for_bone.get_parent() == null else pre_damp * default_dampening
	cos_half_dampen = cos(dampening / 2.0)

	if constraint_state != null:
		constraint = constraint_state.get_direct_reference()
		sim_constraint_swing_axes = sim_transforms[constraint_state.get_swing_transform().get_index()]
		sim_constraint_twist_axes = null if constraint_state.get_twist_transform() == null else sim_transforms[constraint_state.get_twist_transform().get_index()]

		var k: IKKusudama = constraint_state.get_direct_reference() as IKKusudama
		if k != null and k.get_painfulness() > 0.0:
			springy = true
			var iterations: int = for_armature.get_default_iterations()
			half_returnfullness_dampened_iterated.clear()
			half_returnfullness_dampened_iterated.resize(iterations)
			cos_half_returnfullness_dampened_iterated.clear()
			cos_half_returnfullness_dampened_iterated.resize(iterations)
			for i in range(iterations):
				var iterations_clamp: float = compute_iterate_returnfulness(i, for_armature.get_default_iterations(), k)
				var cos_iteration_return_clamp: float = cos(iterations_clamp / 2.0)
				half_returnfullness_dampened_iterated[i] = iterations_clamp
				cos_half_returnfullness_dampened_iterated[i] = cos_iteration_return_clamp
		else:
			springy = false

		if k != null:
			var abl: Array[IKLimitCone] = k.get_limit_cones()
			var cone_count: int = (abl.size() * 3) - 2
			bone_tip_vectors.clear()
			bone_tip_vectors.resize(cone_count)
			cosine_radii_cache.clear()
			cosine_radii_cache.resize(cone_count)
			limit_cone_local_direction_cache.clear()
			limit_cone_local_direction_cache.resize(cone_count)

			for i in range(abl.size()):
				var lc: IKLimitCone = abl[i]
				bone_tip_vectors[i * 3] = Vector3()
				limit_cone_local_direction_cache[i * 3] = lc.get_control_point() * -1.0
				cosine_radii_cache[i * 3] = abl[i].get_radius_cosine()

				if lc.get_tangent_circle_center_next_1(0) != null and i < cone_count - 1:
					bone_tip_vectors[i * 3 + 1] = Vector3()
					bone_tip_vectors[i * 3 + 2] = Vector3()
					limit_cone_local_direction_cache[i * 3 + 1] = lc.get_tangent_circle_center_next_1(0).copy()
					cosine_radii_cache[i * 3 + 1] = 1.0 - lc.get_tangent_circle_radius_next_cos(0)
					limit_cone_local_direction_cache[i * 3 + 2] = lc.get_tangent_circle_center_next_2(0).copy()
					cosine_radii_cache[i * 3 + 2] = 1.0 - lc.get_tangent_circle_radius_next_cos(0)


func fast_update_optimal_rotation_to_pinned_descendants(dampening: float, translate: bool) -> void:
	sim_local_axes.update_global()

	var new_dampening: float
	if translate:
		new_dampening = PI * 2
	else:
		new_dampening = -1

	var localized_target_headings: Array[Vector3] = on_chain.bone_centered_target_headings
	var weights: Array[float] = on_chain.weights
	var hdx: int = 0
	for i in range(on_chain.pinned_bones.size()):
		var sb: WorkingBone = on_chain.pinned_bones[i]
		var target_axes: IKNode3D = sb.sim_target_axes
		target_axes.update_global()
		var origin: Vector3 = sim_local_axes.origin_()
		localized_target_headings[hdx] = target_axes.origin_().sub(origin)
		var mode_code: int = sb.target_state.get_mode_code()
		hdx += 1

		if (mode_code & IKPin.X_DIR) != 0:
			var x_target: IKRay3D = target_axes.x_().get_ray_scaled_by(weights[hdx])
			localized_target_headings[hdx] = x_target.p2 - origin
			# TODO: SUS
			x_target.set_to_inverted_tip(localized_target_headings[hdx + 1]).sub(origin)
			hdx += 2
		if (mode_code & IKPin.Y_DIR) != 0:
			var y_target: IKRay3D = target_axes.y_().get_ray_scaled_by(weights[hdx])
			localized_target_headings[hdx] = y_target.p2 - origin
			# TODO: SUS
			y_target.set_to_inverted_tip(localized_target_headings[hdx + 1]).sub(origin)
			hdx += 2
		if (mode_code & IKPin.Z_DIR) != 0:
			var z_target: IKRay3D = target_axes.z_().get_ray_scaled_by(weights[hdx])
			localized_target_headings[hdx] = z_target.p2 - origin
			# TODO: SUS
			z_target.set_to_inverted_tip(localized_target_headings[hdx + 1]).sub(origin)
			hdx += 2

	var prev_orientation: Quaternion = sim_local_axes.get_local_m_basis().rotation.rotation
	var got_closer: bool = true

	for i in range(for_armature.default_stabilizing_pass_count + 1):
		update_tip_headings(on_chain.bone_centered_tip_headings, true)
		update_optimal_rotation_to_pinned_descendants(new_dampening, translate, on_chain.bone_centered_tip_headings,
														on_chain.bone_centered_target_headings, weights)

		if for_armature.default_stabilizing_pass_count > 0:
			update_tip_headings(on_chain.uniform_bone_centered_tip_headings, false)
			var current_msd: float = on_chain.get_manual_msd(on_chain.uniform_bone_centered_tip_headings,
																on_chain.bone_centered_target_headings,
																on_chain.weights)

			if current_msd <= on_chain.previous_deviation * 1.000001:
				on_chain.previous_deviation = current_msd
				got_closer = true
				break
			else:
				got_closer = false

	if not got_closer:
		sim_local_axes.set_local_orientation_to(prev_orientation)

	if on_chain.wb_segment_root == self:
		on_chain.previous_deviation = INF

	sim_local_axes.mark_dirty()


func pull_back_toward_allowable_region(iteration: int, total_iterations: int) -> void:
	if springy and constraint != null and constraint is IKKusudama:
		constraint.set_axes_to_returnfulled(sim_local_axes, sim_constraint_swing_axes,
											sim_constraint_twist_axes, cos_half_return_damp, return_damp / 2)
		on_chain.previous_deviation = INF


func update_tip_headings(localized_tip_headings: Array[Vector3], scale: bool) -> void:
	var hdx: int = 0

	for i in range(on_chain.pinned_bones.size()):
		var sb: WorkingBone = on_chain.pinned_bones[i]
		var tip_axes: IKNode3D = sb.sim_local_axes
		tip_axes.update_global()
		var origin: Vector3 = sim_local_axes.origin_()
		var target: TargetState = sb.target_state
		var mode_code: int = target.get_mode_code()

		var target_axes: IKNode3D = sb.sim_target_axes
		target_axes.update_global()
		localized_tip_headings[hdx].set(tip_axes.origin_()).sub(origin)

		if scale:
			scale_by = sim_local_axes.origin_().distance_to(target_axes.origin_())
		else:
			scale_by = 1

		hdx += 1

		if (mode_code & IKPin.X_DIR) != 0:
			var x_tip: IKRay3D = tip_axes.x_().get_ray_scaled_by(scale_by)
			localized_tip_headings[hdx].set(x_tip.p2()).sub(origin)
			x_tip.set_to_inverted_tip(localized_tip_headings[hdx + 1]).sub(origin)
			hdx += 2
		if (mode_code & IKPin.Y_DIR) != 0:
			var y_tip: IKRay3D = tip_axes.y_().get_ray_scaled_by(scale_by)
			localized_tip_headings[hdx].set(y_tip.p2()).sub(origin)
			y_tip.set_to_inverted_tip(localized_tip_headings[hdx + 1]).sub(origin)
			hdx += 2
		if (mode_code & IKPin.Z_DIR) != 0:
			var z_tip: IKRay3D = tip_axes.z_().get_ray_scaled_by(scale_by)
			localized_tip_headings[hdx].set(z_tip.p2()).sub(origin)
			z_tip.set_to_inverted_tip(localized_tip_headings[hdx + 1]).sub(origin)
			hdx += 2


func update_cos_dampening() -> void:
	var predamp: float = 1 - for_bone.get_stiffness()
	var default_dampening: float = for_armature.get_dampening()

	if for_bone.get_parent() == null:
		dampening = PI
	else:
		dampening = predamp * default_dampening

	cos_half_dampen = cos(dampening / 2)

	if constraint != null and constraint is IKKusudama:
		var k: IKKusudama = constraint
		if k.get_painfulness() >= 0:
			return_damp = max(default_dampening / 2, k.get_painfulness() / 2)
			cos_half_return_damp = cos(return_damp / 2)
			springy = true
		else:
			springy = false

func compute_iterate_returnfulness(iteration: float, total_iteration: float, k: IKKusudama) -> float:
	var predamp: float = 1 - for_bone.get_stiffness()
	var default_dampening: float = for_armature.get_dampening()

	if for_bone.get_parent() == null:
		dampening = PI
	else:
		dampening = predamp * default_dampening

	var returnfullness: float = k.get_painfulness()
	var falloff: float = 0.2
	var total_itr: float = total_iteration
	var iterations_pow: float = 1 + pow(total_itr, falloff * total_itr * returnfullness)
	var iteration_scalar: float = (iterations_pow - pow(iteration, falloff * total_itr * returnfullness)) / iterations_pow
	var iteration_return_clamp: float = iteration_scalar * returnfullness * dampening
	return iteration_return_clamp
