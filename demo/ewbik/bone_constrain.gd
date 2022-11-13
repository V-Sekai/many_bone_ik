@tool
extends EditorScript
var human_bone: Array  = [
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

func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	var queue : Array
	queue.push_back(root)
	var string_builder : Array
	var skeleton : Skeleton3D
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
	if ewbik != null:
		ewbik.free()
	if skeleton == null:
		return
	skeleton.reset_bone_poses()
	ewbik = NBoneIK.new()
	skeleton.add_child(ewbik, true)
	ewbik.owner = root
	var godot_to_vrm : Dictionary
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	var bone_map : BoneMap = BoneMap.new()
	bone_map.profile = profile
	var bone_vrm_mapping : Dictionary
	ewbik.max_ik_iterations = 10
	var pin_i = 0
	var bones = [
		"Head", 
		"LeftHand", 
		"RightHand", 
		"LeftFoot", 
		"RightFoot"
	]
	ewbik.pin_count = bones.size()
	var head_3d : Marker3D = root.find_child("Head*")
	if head_3d == null:
		head_3d = Marker3D.new()
	for bone_name in bones:
		var bone_index = skeleton.find_bone(bone_name)
		var node_3d : Marker3D = Marker3D.new()
		node_3d.name = bone_name
		node_3d.gizmo_extents = 0.5
		if bone_name == "Head":
			head_3d.name = bone_name
			head_3d = node_3d
			root.add_child(head_3d, true)
			head_3d.owner = root
		else: 
			head_3d.add_child(node_3d, true)
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

		var bone_id = skeleton.find_bone(bone_name)
		if bone_id == -1:
			pin_i = pin_i + 1
			continue
		var bone_transform_relative_to_universe : Transform3D = skeleton.global_transform * skeleton.get_bone_global_rest(bone_id)
		node_3d.transform = node_3d.global_transform.affine_inverse() * bone_transform_relative_to_universe.orthonormalized()
		pin_i = pin_i + 1
	ewbik.set_constraint_count(human_bone.size())
	for bone_i in range(human_bone.size()):
		var bone_name : String = human_bone[bone_i]
		var constraint_i : int = ewbik.find_constraint(bone_name)
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
