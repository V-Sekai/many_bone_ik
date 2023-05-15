@tool
extends Object

class_name ShadowSkeleton

var sim_transforms: Array
var shadow_space: Transform3D
var traversal_array: Array
var root_segment: IKArmatureSegment
var skel_state: SkeletonState
var base_dampening = PI

func _init(skel_state: SkeletonState, base_dampening: float):
	self.skel_state = skel_state
	self.shadow_space = Transform3D();
	self.base_dampening = base_dampening;

	# Build simTransforms hierarchy
	var transform_count = skel_state.get_transform_count()
	if transform_count != 0:
		sim_transforms = []

		for i in range(transform_count):
			var ts = skel_state.get_transform_state(i)
			var new_transform: Transform3D = shadow_space
			new_transform = Transform3D(ts.rotation.scaled(ts.scale), ts.translation)
			sim_transforms.append(new_transform)

		for i in range(transform_count):
			var ts = skel_state.get_transform_state(i)
			var par_ts_idx = ts.get_parent_index()
			var sim_t = sim_transforms[i]
			if par_ts_idx == -1:
				sim_t.set_relative_to_parent(shadow_space)
			else:
				sim_t.set_relative_to_parent(sim_transforms[par_ts_idx])

	# Build armature segment hierarchy
	var root_bone = skel_state.get_root_bone_state()
	if root_bone != null:
		root_segment = IKArmatureSegment.new(self, root_bone, null, false)

	# Build traversal array
	if root_segment != null:
		var segment_traversal_array = root_segment.get_descendant_segments()
		var reversed_traversal_array = []
		for segment in segment_traversal_array:
			reversed_traversal_array.extend(segment.reversed_traversal_array)
		traversal_array = []
		for i in range(reversed_traversal_array.size() - 1, -1, -1):
			traversal_array.append(reversed_traversal_array[i])

func set_dampening(dampening: float) -> void:
	self.base_dampening = dampening
	update_rates()

func solve(iterations: int, stabilization_passes: int, notifier: Callable) -> void:
	var end_on_index = traversal_array.size() - 1
	var transforms = skel_state.get_transforms_array()
	for i in range(transforms.size()):
		sim_transforms[i].get_local_m_basis().set(transforms[i].translation, transforms[i].rotation,
													transforms[i].scale)
		sim_transforms[i]._exclusive_mark_dirty()
	for i in range(iterations):
		for j in range(end_on_index + 1):
			traversal_array[j].pull_back_toward_allowable_region(i, iterations)
		for j in range(end_on_index + 1):
			traversal_array[j].fast_update_optimal_rotation_to_pinned_descendants(base_dampening,
																					j == end_on_index and end_on_index == traversal_array.size() - 1)
	for wb in traversal_array:
		align_bone(wb)

func update_rates() -> void:
	for wb in traversal_array:
		wb.update_cos_dampening()

func align_bone(wb: WorkingBone) -> void:
	var bs = wb.for_bone
	var ts = bs.get_transform()
	ts.translation = wb.sim_local_axes.local_m_basis.translate.get()
	ts.rotation = wb.sim_local_axes.local_m_basis.rotation.to_array()

