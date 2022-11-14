@tool
extends EditorScript
func create_pins(ewbik : NBoneIK, skeleton):
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
	for pin_i in range(pins.size()):
		var pin = pins[pin_i]
		var bone_name = pin
		var bone_i = skeleton.find_bone(bone_name)
		if bone_i == -1:
			continue
		var node_3d : Marker3D = Marker3D.new()
		node_3d.name = bone_name
		node_3d.gizmo_extents = 0.5
		skeleton.add_child(node_3d)
		node_3d.owner = root
		var path_string : String = str(ewbik.get_path_to(root)) + "/" + str(root.get_path_to(node_3d))
		ewbik.set_pin_nodepath(pin_i, NodePath(path_string))
		ewbik.set_pin_bone_name(pin_i, bone_name)
		ewbik.set_pin_depth_falloff(pin_i, 1)
		ewbik.set_pin_weight(pin_i, 0.2)
		if bone_name in ["Head"]:
			ewbik.set_pin_depth_falloff(pin_i, 1)
		if bone_name in ["LeftFoot", "RightFoot"]:
			ewbik.set_pin_weight(pin_i, 0.4)
		var bone_transform : Transform3D = skeleton.get_bone_global_rest(bone_i)
		node_3d.global_transform = bone_transform
func create_constraints(ewbik, skeleton):
	var human_bones: PackedStringArray  = [
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

	var pending_bones : PackedStringArray
	for bone_name in human_bones:
		if skeleton.find_bone(bone_name) != -1:
			pending_bones.push_back(bone_name)
	human_bones.clear()
	human_bones.append_array(pending_bones)
	ewbik.set_constraint_count(human_bones.size())
	var constraint_i = 0
	while constraint_i < human_bones.size():
		var bone_name = human_bones[constraint_i]
		ewbik.set_constraint_name(constraint_i, bone_name)
#		# https://pubmed.ncbi.nlm.nih.gov/32644411/
		if bone_name in ["Chest"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(10))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-10), deg_to_rad(10)))
		elif bone_name in ["UpperChest"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-10), deg_to_rad(10)))
		elif bone_name in ["Hips"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(5), deg_to_rad(-5)))
		elif bone_name in ["Spine"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
		elif bone_name in ["Head"]:
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
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(-1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(5), deg_to_rad(-5)))
		elif bone_name in ["LeftShoulder"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(5), deg_to_rad(-5)))
		elif bone_name in ["RightUpperArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(60))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-90), deg_to_rad(90)))
		elif bone_name in ["LeftUpperArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(-1, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(60))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(90), deg_to_rad(-90)))
		elif bone_name in ["RightLowerArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(20))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(-10)))
		elif bone_name in ["LeftLowerArm"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(-1, 0, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(20))
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
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(90))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
		elif bone_name in ["LeftUpperLeg"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(90))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
		elif bone_name in ["RightLowerLeg"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
		elif bone_name in ["LeftLowerLeg"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 3)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			ewbik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-360),  deg_to_rad(360)))
		elif bone_name in ["RightFoot"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0),  deg_to_rad(130)))
		elif bone_name in ["LeftFoot"]:
			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0),  deg_to_rad(130)))
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
			ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(-270),  deg_to_rad(270)))
		else:
			print(bone_name)
		constraint_i = constraint_i + 1

func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var queue : Array
	queue.push_back(root)
	var string_builder : Array
	var skeleton : Skeleton3D = null
	var ewbik : NBoneIK = null
	while not queue.is_empty():
		var front = queue.front()
		var node : Node = front
		if node is Skeleton3D:
			skeleton = node
		if node is NBoneIK:
			ewbik = node
		var child_count : int = node.get_child_count()
		for i in child_count:
			queue.push_back(node.get_child(i))
		queue.pop_front()
	if skeleton == null:
		return
	if ewbik == null:
		ewbik = NBoneIK.new()
		skeleton.reset_bone_poses()
		skeleton.add_child(ewbik, true)
		ewbik.owner = root
	ewbik.max_ik_iterations = 10
	create_pins(ewbik, skeleton)
	create_constraints(ewbik, skeleton)
	
