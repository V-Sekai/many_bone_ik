# MIT License
#
# Copyright (c) 2020 K. S. Ernest (iFire) Lee & V-Sekai
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# GDScript implementation of a modified version of the fortune algorthim.
# Thanks to iFire, Lyuma, and MMMaellon for the C++ implementation this version was based off, saved me
# a lot of headaches ;)
# https://github.com/godot-extended-libraries/godot-fire/commit/622022d2779f9d35b586db4ee31c9cb76d0b7bc7

@tool
extends EditorScript
func create_pins(ewbik : NBoneIK, skeleton : Skeleton3D):
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var godot_to_vrm : Dictionary
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	var bone_map : BoneMap = BoneMap.new()
	bone_map.profile = profile
	var bone_vrm_mapping : Dictionary
	var pins =  [
		"Head",
		"LeftHand", 
		"RightHand", 
		"LeftFoot", 
		"RightFoot"
	]
	for pin in pins:
		var node = root.find_child(pin)
		if node != null:
			node.free()
	ewbik.set_pin_count(pins.size())
	ewbik.enable = false
	skeleton.reset_bone_poses()
	for pin_i in range(pins.size()):
		var pin = pins[pin_i]
		var bone_name = pin
		var bone_i = skeleton.find_bone(bone_name)
		if bone_i == -1:
			continue
		var node_3d : BoneAttachment3D = BoneAttachment3D.new()
		node_3d.name = bone_name
		node_3d.bone_name = bone_name
		node_3d.bone_idx = skeleton.find_bone(bone_name)
		node_3d.set_use_external_skeleton(true)
		node_3d.set_external_skeleton(str("../") + str(root.get_path_to(skeleton)))
		root.add_child(node_3d)
		node_3d.owner = root
		var path_string : String = str(ewbik.get_path_to(root)) + "/" + str(root.get_path_to(node_3d))
		ewbik.set_pin_nodepath(pin_i, NodePath(path_string))
		ewbik.set_pin_bone_name(pin_i, bone_name)
		ewbik.set_pin_depth_falloff(pin_i, 1)
		ewbik.set_pin_weight(pin_i, 0.2)
		if bone_name in ["Head"]:
			ewbik.set_pin_weight(pin_i, 1)
			ewbik.set_pin_depth_falloff(pin_i, 0)
		if bone_name in ["LeftFoot", "RightFoot"]:
			ewbik.set_pin_weight(pin_i, 0.4)
		var marker_3d : Marker3D = Marker3D.new()
		marker_3d.name = bone_name
		marker_3d.global_transform = node_3d.global_transform
		node_3d.replace_by(marker_3d, true)
		marker_3d.gizmo_extents = 0.5
	ewbik.enable = true
	
const human_bones: PackedStringArray  = [
	"Hips",
	"Spine",
	"Chest",
	"UpperChest",
	"Neck",
	"Head",
	"LeftEye",
	"RightEye",
	"Jaw",
	"LeftShoulder",
	"LeftUpperArm",
	"LeftLowerArm",
	"LeftHand",
	"LeftThumbMetacarpal",
	"LeftThumbProximal",
	"LeftThumbDistal",
	"LeftIndexProximal",
	"LeftIndexIntermediate",
	"LeftIndexDistal",
	"LeftMiddleProximal",
	"LeftMiddleIntermediate",
	"LeftMiddleDistal",
	"LeftRingProximal",
	"LeftRingIntermediate",
	"LeftRingDistal",
	"LeftLittleProximal",
	"LeftLittleIntermediate",
	"LeftLittleDistal",
	"RightShoulder",
	"RightUpperArm",
	"RightLowerArm",
	"RightHand",
	"RightThumbMetacarpal",
	"RightThumbProximal",
	"RightThumbDistal",
	"RightIndexProximal",
	"RightIndexIntermediate",
	"RightIndexDistal",
	"RightMiddleProximal",
	"RightMiddleIntermediate",
	"RightMiddleDistal",
	"RightRingProximal",
	"RightRingIntermediate",
	"RightRingDistal",
	"RightLittleProximal",
	"RightLittleIntermediate",
	"RightLittleDistal",
	"LeftUpperLeg",
	"LeftLowerLeg",
	"LeftFoot",
	"LeftToes",
	"RightUpperLeg",
	"RightLowerLeg",
	"RightFoot",
	"RightToes"
]

# NEED TO CHECK FOR PARENT TO CHILDREN BONE Y UP
func create_constraints(ewbik, skeleton):
	var pending_bones : PackedStringArray
	for bone_name in human_bones:
		if skeleton.find_bone(bone_name) != -1:
			pending_bones.push_back(bone_name)
	human_bones.clear()
	human_bones.append_array(pending_bones)
	ewbik.set_constraint_count(human_bones.size())
	for constraint_i in range(human_bones.size()):
		var bone_name = human_bones[constraint_i]
		ewbik.set_constraint_name(constraint_i, bone_name)
#		# https://pubmed.ncbi.nlm.nih.gov/32644411/
		ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-270),  deg_to_rad(270)))
		if bone_name in ["Chest"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(10))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-10), deg_to_rad(10)))
		elif bone_name in ["UpperChest"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(30))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-10), deg_to_rad(10)))
		elif bone_name in ["Hips"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-270),  deg_to_rad(270)))
		elif bone_name in ["Spine"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
		elif bone_name in ["Head"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(35))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(180), deg_to_rad(-180)))
		elif bone_name in ["Neck"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(180), deg_to_rad(-180)))
		elif bone_name in ["Spine"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(10))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-1), deg_to_rad(1)))
		elif bone_name in ["RightShoulder",]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(5), deg_to_rad(-5)))
		elif bone_name in ["LeftShoulder"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(5), deg_to_rad(-5)))
		elif bone_name in ["RightUpperArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(30))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-90), deg_to_rad(90)))
		elif bone_name in ["LeftUpperArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(30))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(90), deg_to_rad(-90)))
		elif bone_name in ["RightLowerArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(10))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(10))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(10))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(-10)))
		elif bone_name in ["LeftLowerArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(10))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(-1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(10))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(10))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(10)))
		elif bone_name in ["RightHand"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(90))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-20), deg_to_rad(20)))
		elif bone_name in ["LeftHand"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(90))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-20), deg_to_rad(20)))
		elif bone_name in ["RightUpperLeg"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(160))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
		elif bone_name in ["LeftUpperLeg"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(160))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
		elif bone_name in ["RightLowerLeg"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 2)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(30))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
		elif bone_name in ["LeftLowerLeg"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 2)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(30))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
#		elif bone_name in ["RightFoot"]:
#			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
#			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
#			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
#			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0),  deg_to_rad(130)))
#		elif bone_name in ["LeftFoot"]:
#			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
#			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
#			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
#			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0),  deg_to_rad(130)))
		elif bone_name in ["RightToes"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0),  deg_to_rad(130)))
		elif bone_name in ["LeftToes"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0),  deg_to_rad(130)))
		elif bone_name in [
			"LeftEye",
			"RightEye",
			"Jaw",
			"LeftThumbMetacarpal",
			"LeftThumbProximal",
			"LeftThumbDistal",
			"LeftIndexProximal",
			"LeftIndexIntermediate",
			"LeftIndexDistal",
			"LeftMiddleProximal",
			"LeftMiddleIntermediate",
			"LeftMiddleDistal",
			"LeftRingProximal",
			"LeftRingIntermediate",
			"LeftRingDistal",
			"LeftLittleProximal",
			"LeftLittleIntermediate",
			"LeftLittleDistal",
			"RightThumbMetacarpal",
			"RightThumbProximal",
			"RightThumbDistal",
			"RightIndexProximal",
			"RightIndexIntermediate",
			"RightIndexDistal",
			"RightMiddleProximal",
			"RightMiddleIntermediate",
			"RightMiddleDistal",
			"RightRingProximal",
			"RightRingIntermediate",
			"RightRingDistal",
			"RightLittleProximal",
			"RightLittleIntermediate",
			"RightLittleDistal"
			]:
			pass
		else:
			print(bone_name)


# MIT License
#
# Copyright (c) 2020 K. S. Ernest (iFire) Lee & V-Sekai
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# GDScript implementation of a modified version of the fortune algorthim.
# Thanks to iFire, Lyuma, and MMMaellon for the C++ implementation this version was based off, saved me
# a lot of headaches ;)
# https://github.com/godot-extended-libraries/godot-fire/commit/622022d2779f9d35b586db4ee31c9cb76d0b7bc7

const NO_BONE = -1
const VECTOR_DIRECTION = Vector3.UP

class RestBone extends RefCounted:
	var rest_local_before: Transform3D
	var rest_local_after: Transform3D
	var rest_delta: Quaternion
	var children_centroid_direction: Vector3
	var parent_index: int = NO_BONE
	var children: Array = []
	var override_direction: bool = true


static func _align_vectors(a: Vector3, b: Vector3) -> Quaternion:
	a = a.normalized()
	b = b.normalized()
	var angle: float = a.angle_to(b)
	if is_zero_approx(angle):
		return Quaternion()
	if !is_zero_approx(a.length_squared()) and !is_zero_approx(b.length_squared()):
		# Find the axis perpendicular to both vectors and rotate along it by the angular difference
		var perpendicular: Vector3 = a.cross(b).normalized()
		var angle_diff: float = a.angle_to(b)
		if is_zero_approx(perpendicular.length_squared()):
			perpendicular = Vector3()
			if !is_zero_approx(a[0]) and !is_zero_approx(a[1]):
				perpendicular = Vector3(0, 0, 1).cross(a).normalized()
			else:
				perpendicular = Vector3(1, 0, 0)
		return Quaternion(perpendicular, angle_diff)
	else:
		return Quaternion()

static func _fortune_with_chains(
	p_skeleton: Skeleton3D,
	r_rest_bones: Dictionary,
	p_fixed_chains: Array,
	p_ignore_unchained_bones: bool,
	p_ignore_chain_tips: Array,
	p_base_pose: Array) -> Dictionary:
	var bone_count: int = p_skeleton.get_bone_count()

	# First iterate through all the bones and create a RestBone for it with an empty centroid
	for j in range(0, bone_count):
		var rest_bone: RestBone = RestBone.new()

		rest_bone.parent_index = p_skeleton.get_bone_parent(j)
		rest_bone.rest_local_before = p_base_pose[j]
		rest_bone.rest_local_after = rest_bone.rest_local_before
		r_rest_bones[j] = rest_bone

	# Collect all bone chains into a hash table for optimisation
	var chain_hash_table: Dictionary = {}.duplicate()
	for chain in p_fixed_chains:
		for bone_id in chain:
			chain_hash_table[bone_id] = chain

	# We iterate through again, and add the child's position to the centroid of its parent.
	# These position are local to the parent which means (0, 0, 0) is right where the parent is.
	for i in range(0, bone_count):
		var parent_bone: int = p_skeleton.get_bone_parent(i)
		if (parent_bone >= 0):

			var apply_centroid = true

			var chain = chain_hash_table.get(parent_bone, null)
			if typeof(chain) == TYPE_PACKED_INT32_ARRAY:
				var index: int = NO_BONE
				for findind in range(len(chain)):
					if chain[findind] == parent_bone:
						index = findind
				if (index + 1) < chain.size():
					# Check if child bone is the next bone in the chain
					if chain[index + 1] == i:
						apply_centroid = true
					else:
						apply_centroid = false
				else:
					# If the bone is at the end of a chain, p_ignore_chain_tips argument determines
					# whether it should attempt to be corrected or not
					if p_ignore_chain_tips.has(chain):
						r_rest_bones[parent_bone].override_direction = false
						apply_centroid = false
					else:
						apply_centroid = true
			else:
				if p_ignore_unchained_bones:
					r_rest_bones[parent_bone].override_direction = false
					apply_centroid = false
			if apply_centroid:
				r_rest_bones[parent_bone].children_centroid_direction = r_rest_bones[parent_bone].children_centroid_direction + p_skeleton.get_bone_rest(i).origin
			r_rest_bones[parent_bone].children.append(i)


	# Point leaf bones to parent
	for i in range(0, bone_count):
		var leaf_bone: RestBone = r_rest_bones[i]
		if (leaf_bone.children.size() == 0):
			if p_ignore_unchained_bones and !chain_hash_table.get(i, null):
				r_rest_bones[i].override_direction = false
			leaf_bone.children_centroid_direction = r_rest_bones[leaf_bone.parent_index].children_centroid_direction

	# We iterate again to point each bone to the centroid
	# When we rotate a bone, we also have to move all of its children in the opposite direction
	for i in range(0, bone_count):
		if r_rest_bones[i].override_direction:
			r_rest_bones[i].rest_delta = _align_vectors(VECTOR_DIRECTION, r_rest_bones[i].children_centroid_direction)
			r_rest_bones[i].rest_local_after.basis = r_rest_bones[i].rest_local_after.basis * Basis(r_rest_bones[i].rest_delta)

			# Iterate through the children and rotate them in the opposite direction.
			for j in range(0, r_rest_bones[i].children.size()):
				var child_index: int = r_rest_bones[i].children[j]
				r_rest_bones[child_index].rest_local_after = Transform3D(r_rest_bones[i].rest_delta.inverse(), Vector3()) * r_rest_bones[child_index].rest_local_after

	return r_rest_bones


static func find_mesh_instances_for_avatar_skeleton(p_node: Node, p_skeleton: Skeleton3D, p_valid_mesh_instances: Array) -> Array:
	if p_skeleton and p_node is MeshInstance3D:
		var skeleton: Node = p_node.get_node_or_null(p_node.skeleton)
		if skeleton == p_skeleton:
			p_valid_mesh_instances.push_back(p_node)

	for child in p_node.get_children():
		p_valid_mesh_instances = find_mesh_instances_for_avatar_skeleton(child, p_skeleton, p_valid_mesh_instances)

	return p_valid_mesh_instances


static func get_fortune_with_chain_offsets(p_skeleton: Skeleton3D, p_base_pose: Array) -> Dictionary:
	var rest_bones: Dictionary = _fortune_with_chains(p_skeleton, {}.duplicate(), [], false, [], p_base_pose)

	var offsets: Dictionary = {"base_pose_offsets":[], "bind_pose_offsets":[]}

	for key in rest_bones.keys():
		offsets["base_pose_offsets"].append(rest_bones[key].rest_local_before.inverse() * rest_bones[key].rest_local_after)
		offsets["bind_pose_offsets"].append(Transform3D(rest_bones[key].rest_delta.inverse()))

	return offsets

func _post_process(p_root: Node) -> void:
	var queue : Array
	queue.push_back(p_root)
	var string_builder : Array
	while not queue.is_empty():
		var front = queue.front()
		var node = front
		if node is Skeleton3D:
			print("bone_direction: fix_skeleton")
			var base_pose: Array = []
			for i in range(0, node.get_bone_count()):
				base_pose.append(node.get_bone_rest(i))
			var offsets: Dictionary = get_fortune_with_chain_offsets(node, base_pose)
			for i in range(0, offsets["base_pose_offsets"].size()):
				var final_pose: Transform3D = node.get_bone_rest(i) * offsets["base_pose_offsets"][i]
				var old_scale: Vector3 = node.get_bone_pose_scale(i)
				var new_rotation: Quaternion = Quaternion(final_pose.basis.orthonormalized())
				node.set_bone_pose_position(i, final_pose.origin)
				node.set_bone_pose_scale(i, old_scale)
				node.set_bone_pose_rotation(i, new_rotation)
				node.set_bone_rest(i, Transform3D(
						Basis(new_rotation) * Basis(Vector3(1,0,0) * old_scale.x, Vector3(0,1,0) * old_scale.y, Vector3(0,0,1) * old_scale.z),
						final_pose.origin))
			# Correct the bind poses
			var mesh_instances: Array = find_mesh_instances_for_avatar_skeleton(p_root, node, [])
			print("bone_direction: _fix_meshes")
			for mi in mesh_instances:
				var skin: Skin = mi.get_skin();
				if skin == null:
					continue

				skin = skin.duplicate()
				mi.set_skin(skin)
				var skeleton_path: NodePath = mi.get_skeleton_path()
				var skeleton_node: Node = mi.get_node_or_null(skeleton_path)
				var skeleton: Skeleton3D = skeleton_node
				for bind_i in range(0, skin.get_bind_count()):
					var bone_index:int  = skin.get_bind_bone(bind_i)
					if (bone_index == NO_BONE):
						var bind_name: String = skin.get_bind_name(bind_i)
						if bind_name.is_empty():
							continue
						bone_index = skeleton.find_bone(bind_name)

					if (bone_index == NO_BONE):
						continue
					skin.set_bind_pose(bind_i, offsets["bind_pose_offsets"][bone_index] * skin.get_bind_pose(bind_i))

			print("Refresh the skeleton.")
			node.visible = not node.visible
			node.visible = not node.visible
		var child_count : int = node.get_child_count()
		for i in child_count:
			queue.push_back(node.get_child(i))
		queue.pop_front()
	return p_root

func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	var skeleton : Skeleton3D = null
	for new_skeleton in skeletons:
		skeleton = new_skeleton
		break
	var iks : Array[Node] = root.find_children("*", "NBoneIK")
	var ik : NBoneIK = null
	for new_ik in iks:
		ik = new_ik
		break
	if skeleton == null:
		return
	if ik == null:
		ik = NBoneIK.new()
		root.add_child(ik, true)
		ik.owner = root
	ik.skeleton_node_path = "../" + str(root.get_path_to(skeleton))
	ik.max_ik_iterations = 10
#	_post_process(skeleton)
	create_pins(ik, skeleton)
	create_constraints(ik, skeleton)
	ik.queue_print_skeleton()
