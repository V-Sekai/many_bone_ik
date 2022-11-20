@tool
extends EditorScript

func enable_debug_pins(pins, ewbik : NBoneIK):
	for pin_i in range(pins.size()):
		ewbik.set_pin_weight(pin_i, 0)
		if str(pins[pin_i]) == "Root":
			ewbik.set_pin_weight(pin_i, 1)
		ewbik.default_damp = 0.001

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
		"Spine",
		"Root",
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
		if bone_name in ["Hips"]:
			ewbik.set_pin_passthrough_factor(pin_i, 0.1)
		var node_3d : BoneAttachment3D = BoneAttachment3D.new()
		node_3d.name = bone_name
		node_3d.bone_name = bone_name
		node_3d.bone_idx = skeleton.find_bone(bone_name)
		skeleton.add_child(node_3d)
		node_3d.owner = root
		var path_string : String = "../" + str(skeleton.get_path_to(node_3d))
		ewbik.set_pin_nodepath(pin_i, NodePath(path_string))
		ewbik.set_pin_bone_name(pin_i, bone_name)
		ewbik.set_pin_passthrough_factor(pin_i, 1)			
		if bone_name in ["UpperChest"]:
			ewbik.set_pin_weight(pin_i, 0.01)
		var marker_3d : Marker3D = Marker3D.new()
		marker_3d.name = bone_name
		marker_3d.global_transform = node_3d.global_transform
		node_3d.replace_by(marker_3d, true)
		marker_3d.gizmo_extents = 0.1
#	enable_debug_pins(pins, ewbik)
var human_bones: PackedStringArray

func _run():
	# Check if bones roll point forward
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	for bone_i in profile.bone_size:
		human_bones.push_back(profile.get_bone_name(bone_i)) 
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var iks : Array[Node] = root.find_children("*", "NBoneIK")
	for ik in iks:
		var ewbik : NBoneIK = ik
		if ewbik == null:
			continue
		for constraint_i in human_bones.size():
			var bone_name = human_bones[constraint_i]
			var twist_current : float = ewbik.get_kusudama_twist_current(constraint_i)
			var twist_min : float = ewbik.get_kusudama_twist(constraint_i).x
			var twist_range : float = ewbik.get_kusudama_twist(constraint_i).y
			var current = ewbik.get_kusudama_twist_current(constraint_i) 
			if current > 0.1:
				ewbik.set_kusudama_twist(constraint_i, Vector2(lerp_angle(0, TAU - twist_min, current), lerp_angle(0, twist_range, current)))
			else:
				ewbik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(1)))
	
		for constraint_i in human_bones.size():
			var bone_name = human_bones[constraint_i]
			var twist_min = ewbik.get_kusudama_twist(constraint_i).x
			if bone_name in ["Head"]:
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			elif bone_name in ["Neck"]:
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			elif bone_name in ["UpperChest"]:
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
				ewbik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
			elif bone_name in ["Chest"]:
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
				ewbik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
			elif bone_name in ["Spine"]:
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(0.01))
				ewbik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
			elif bone_name in ["Hips"]:
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(0.01))
			elif bone_name.ends_with("Shoulder"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(-1, 0, 0))
				if bone_name.begins_with("Left"):
					ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(1, 0, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(30))
				ewbik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
			elif bone_name.ends_with("UpperArm"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
				ewbik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
			elif bone_name.ends_with("LowerArm"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
				ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(1, 0, 0))
				if bone_name.begins_with("Left"):
					ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(-1, 0, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(1))
				ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(1))
				ewbik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
			elif bone_name.ends_with("Hand"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			elif bone_name.ends_with("UpperLeg"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			elif bone_name.ends_with("LowerLeg"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
				ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(1))
				ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(1))
			elif bone_name.ends_with("Foot"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 2)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
				ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
			elif bone_name.ends_with("Toes"):
				ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
				ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
				ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
