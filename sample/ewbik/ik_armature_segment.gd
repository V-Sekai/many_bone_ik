@tool
extends Object

class_name IKArmatureSegment

## A segment is defined as any set of bones all solving for the same targets.
## A segment may have subsegments, which are all bones solving for one or more strict subsets
## of the targets the segment solves for.
## A segment may have child segments, which do not solve for any of the same targets as the parent segment,
## but whose bones are all descendants of the bones in the parent segment.

var shadow_skel: ShadowSkeleton
var sim_transforms: Array
var for_armature: Skeleton3D
var sub_segments: Array = []
var child_segments: Array = []
var solvable_strand_bones: Array = []
var all_strand_bones: Array = []
var solvable_segment_bones: Array = []
var all_segment_bones: Array = []
var reversed_traversal_array: Array

var wb_segment_root: WorkingBone
var bone_centered_target_headings: Array
var bone_centered_tip_headings: Array
var uniform_bone_centered_tip_headings: Array
var weights: PackedFloat64Array
var pinned_bones: Array[WorkingBone]
var is_root_pinned: bool = false
var has_pinned_ancestor: bool = false
var previous_deviation: float = INF
const DOUBLE_ROUNDING_ERROR = 0.000000000000001
var qcp_converger: QCP = QCP.new(DOUBLE_ROUNDING_ERROR, DOUBLE_ROUNDING_ERROR)
var wb_segment_tip: WorkingBone
var parent_segment: IKArmatureSegment

func _init(shadow_skel: ShadowSkeleton, starting_from: BoneState, parent_segment: IKArmatureSegment, is_root_pinned: bool):
	self.shadow_skel = shadow_skel
	self.sim_transforms = shadow_skel.sim_transforms
	self.for_armature = shadow_skel.for_armature
	self.parent_segment = parent_segment

	self.is_root_pinned = is_root_pinned
	self.has_pinned_ancestor = parent_segment != null && (parent_segment.is_root_pinned || parent_segment.has_pinned_ancestor)

	# build_segment
	var seg_effectors: Array = []
	var strand_bones: Array = []
	var sub_sgmts: Array = []
	var child_sgmts: Array = []
	var current_bs: BoneState = starting_from
	var finished: bool = false

	while not finished:
		var current_wb: WorkingBone = WorkingBone.new(current_bs, self, shadow_skel.sim_transforms, for_armature)
		if current_bs == starting_from:
			self.wb_segment_root = current_wb
		strand_bones.append(current_wb)

		var target: TargetState = current_bs.get_target()
		if target != null or current_bs.get_child_count() > 1:
			if target != null:
				seg_effectors.append(current_wb)
				if target.get_depth_fall_off() <= 0.0:
					self.wb_segment_tip = current_wb
					finished = true

			if finished:
				for i in range(current_bs.get_child_count()):
					child_sgmts.append(IKArmatureSegment.new(shadow_skel, current_bs.get_child(i), self, true))
			else:
				for i in range(current_bs.get_child_count()):
					var subseg: IKArmatureSegment = IKArmatureSegment.new(shadow_skel, current_bs.get_child(i), self, false)
					sub_sgmts.append(subseg)
					sub_sgmts += subseg.sub_segments
					seg_effectors += subseg.pinned_bones
				finished = true
				self.wb_segment_tip = current_wb
		elif current_bs.get_child_count() == 1:
			current_bs = current_bs.get_child(0)
		else:
			self.wb_segment_tip = current_wb

	self.sub_segments = sub_sgmts
	self.pinned_bones = seg_effectors
	self.child_segments = child_sgmts
	self.solvable_strand_bones = strand_bones

	if self.is_root_pinned:
		self.wb_segmentRoot.set_as_segment_root()

	# build_reverse_traversal_array
	var reverse_traversal_array: Array = []

	for wb in solvable_strand_bones:
		if wb.for_bone.get_stiffness() < 1.0:
			reverse_traversal_array.append(wb)

	for ss in sub_segments:
		reverse_traversal_array += ss.reversed_traversal_array

	self.reversed_traversal_array = reverse_traversal_array

	# create_heading_arrays
	var penalty_array: Array = []
	var pin_sequence: Array = []  # TODO: remove after debugging
	recursively_create_penalty_array(penalty_array, pin_sequence, 1.0)
	var total_headings: int = 0

	for a in penalty_array:
		total_headings += len(a)

	bone_centered_target_headings.clear()
	bone_centered_target_headings.resize(total_headings)
	bone_centered_tip_headings.clear()
	bone_centered_tip_headings.resize(total_headings)
	uniform_bone_centered_tip_headings.clear()
	uniform_bone_centered_tip_headings.resize(total_headings)
	weights.clear()
	weights.resize(total_headings)
	var current_heading: int = 0

	for a in penalty_array:
		for ad in a:
			weights[current_heading] = ad
			bone_centered_target_headings[current_heading] = Vector3()
			bone_centered_tip_headings[current_heading] = Vector3()
			uniform_bone_centered_tip_headings[current_heading] = Vector3()
			current_heading += 1

func get_dapening() -> float:
	return shadow_skel.base_dampening

func recursively_create_penalty_array(weight_array: Array, pin_sequence: Array, current_falloff: float) -> void:
	if current_falloff == 0:
		return
	else:
		var target = wb_segment_tip.target_state
		if target != null:
			var inner_weight_array = []
			weight_array.append(inner_weight_array)
			var mode_code = target.get_mode_code()
			inner_weight_array.append(target.get_weight() * current_falloff)
			var max_pin_weight = target.get_max_priority()
			if max_pin_weight == 0:
				max_pin_weight = 1

			if (mode_code & IKPin.X_DIR) != 0:
				var sub_target_weight = target.get_weight() * (target.get_priority(IKPin.X_DIR) / max_pin_weight) * current_falloff
				inner_weight_array.append(sub_target_weight)
				inner_weight_array.append(sub_target_weight)

			if (mode_code & IKPin.Y_DIR) != 0:
				var sub_target_weight = target.get_weight() * (target.get_priority(IKPin.Y_DIR) / max_pin_weight) * current_falloff
				inner_weight_array.append(sub_target_weight)
				inner_weight_array.append(sub_target_weight)

			if (mode_code & IKPin.Z_DIR) != 0:
				var sub_target_weight = target.get_weight() * (target.get_priority(IKPin.Z_DIR) / max_pin_weight) * current_falloff
				inner_weight_array.append(sub_target_weight)
				inner_weight_array.append(sub_target_weight)

			pin_sequence.append(wb_segment_tip)

		var this_falloff = 1 if target == null else target.get_depth_fall_off()
		for s in sub_segments:
			s.recursively_create_penalty_array(weight_array, pin_sequence, current_falloff * this_falloff)

# Get descendant segments
func get_descendant_segments() -> Array:
	var result = []
	result.append(self)
	for child in child_segments:
		result.extend(child.get_descendant_segments())
	return result

# Get manual MSD
func get_manual_msd(loc_tips: Array, loc_targets: Array, weights: Array) -> float:
	var manual_rmsd = 0.0
	var wsum = 0.0
	for i in range(loc_targets.size()):
		var xd = loc_targets[i].x - loc_tips[i].x
		var yd = loc_targets[i].y - loc_tips[i].y
		var zd = loc_targets[i].z - loc_tips[i].z
		var magsq = weights[i] * (xd * xd + yd * yd + zd * zd)
		manual_rmsd += magsq
		wsum += weights[i]

	manual_rmsd /= wsum * wsum
	return manual_rmsd
