@tool
extends EditorScript

func enable_debug_pins(pins, ewbik):
	for pin_i in range(pins.size()):
		ewbik.set_pin_weight(pin_i, 0.01)
		
func create_pins(ewbik : NBoneIK, skeleton : Skeleton3D):	
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	var bone_map : BoneMap = BoneMap.new()
	bone_map.profile = profile
	var pins =  [
		"Head",
		"LeftHand", 
		"RightHand", 
		"LeftFoot", 
		"RightFoot",
	]
	for pin in pins:
		var node = root.find_child(pin)
		if node != null:
			node.free()
	ewbik.set_pin_count(pins.size())
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
		skeleton.add_child(node_3d)
		node_3d.owner = root
		var path_string : String = "../" + str(skeleton.get_path_to(node_3d))
		ewbik.set_pin_nodepath(pin_i, NodePath(path_string))
		ewbik.set_pin_bone_name(pin_i, bone_name)
		ewbik.set_pin_depth_falloff(pin_i, 1)
		if bone_name in ["UpperChest"]:
			ewbik.set_pin_weight(pin_i, 0.01)
		var marker_3d : Marker3D = Marker3D.new()
		marker_3d.name = bone_name
		marker_3d.global_transform = node_3d.global_transform
		node_3d.replace_by(marker_3d, true)
		marker_3d.gizmo_extents = 0.1
#	enable_debug_pins(pins, ewbik)
var human_bones: PackedStringArray


# Create a data structure for constraint generation.

func create_constraints(ewbik):
	ewbik.set_constraint_count(human_bones.size())
	for constraint_i in range(human_bones.size()):
		var bone_name = human_bones[constraint_i]
		ewbik.set_constraint_name(constraint_i, bone_name)
		# https://pubmed.ncbi.nlm.nih.gov/32644411/
		if bone_name in ["Head"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(2))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name in ["Neck"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name in ["RightShoulder",]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(-1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(.1)))
		elif bone_name in ["LeftShoulder"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(.1)))
		elif bone_name.ends_with("UpperArm"):
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
			var twist : Vector2 = Vector2(deg_to_rad(0), deg_to_rad(2))
			ewbik.set_kusudama_twist(constraint_i, twist)
		elif bone_name.ends_with("LowerArm"):
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(-1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(5))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(5))
			if bone_name.begins_with("Left"):
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
				ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(1, 0, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(5))
				ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(5))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(1)))
		elif bone_name.ends_with("Hand"):
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			var twist : Vector2 = Vector2(deg_to_rad(0), deg_to_rad(0.1))
			ewbik.set_kusudama_twist(constraint_i, twist)
		elif bone_name in ["UpperChest"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name in ["Chest"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name in ["Spine"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(0.01))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name in ["Hips"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(0.01))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name.ends_with("UpperLeg"):
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name.ends_with("LowerLeg"):
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(2))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(2))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(2))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(0.1)))
		elif bone_name.ends_with("Foot"):
			ewbik.set_kusudama_limit_cone_count(constraint_i, 2)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(2)))
		elif bone_name.ends_with("Toes"):
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(2)))

func _run():
	# Check if bones roll point forward
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	for bone_i in profile.bone_size:
		human_bones.push_back(profile.get_bone_name(bone_i)) 
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	for skeleton in skeletons:
		var iks : Array[Node] = skeleton.find_children("*", "NBoneIK")
		for ik in iks:
			ik.free()
		var new_ik : NBoneIK = NBoneIK.new()
		skeleton.add_child(new_ik, true)
		new_ik.owner = root
		new_ik.visible = false
		new_ik.iterations_per_frame = 30
		new_ik.set_pin_count(0)
		new_ik.set_constraint_count(0)
		create_constraints(new_ik)
		create_pins(new_ik, skeleton)
		new_ik.visible = true
		new_ik.queue_print_skeleton()
		break
