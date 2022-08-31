@tool
extends EditorScript

func _lock_rotation(ewbik, constraint_i):
	ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
	ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
	ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(90))

func _full_rotation(ewbik, constraint_i):
	ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
	ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
	ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, TAU)

func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	var queue : Array
	queue.push_back(root)
	var string_builder : Array
	var vrm_top_level : Node3D
	var skeleton : Skeleton3D
	var ewbik : EWBIK = null
	while not queue.is_empty():
		var front = queue.front()
		var node : Node = front
		if node is Skeleton3D:
			skeleton = node
		if node.script and node.script.resource_path == "res://addons/vrm/vrm_toplevel.gd":
			vrm_top_level = node
		if node is EWBIK:
			ewbik = node
		var child_count : int = node.get_child_count()
		for i in child_count:
			queue.push_back(node.get_child(i))
		queue.pop_front()
	if ewbik != null:
		ewbik.queue_free()
	ewbik = EWBIK.new()
	skeleton.add_child(ewbik, true)
	ewbik.owner = skeleton.owner
	ewbik.name = "EWBIK"
	ewbik.skeleton = NodePath("..")
	var godot_to_vrm : Dictionary
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	var bone_map : BoneMap = BoneMap.new()
	bone_map.profile = profile
	_generate_ewbik(vrm_top_level, skeleton, ewbik, profile)
	

func _generate_ewbik(vrm_top_level : Node3D, skeleton : Skeleton3D, ewbik : EWBIK, profile : SkeletonProfileHumanoid) -> void:
	var vrm_meta = vrm_top_level.get("vrm_meta")
	var bone_vrm_mapping : Dictionary
	ewbik.max_ik_iterations = 30
	ewbik.default_damp = deg_to_rad(1)
	ewbik.budget_millisecond = 2
	ewbik.live_preview = true
	var minimum_twist = deg_to_rad(-0.5)
	var minimum_twist_diff = deg_to_rad(0.5)
	var maximum_twist = deg_to_rad(360)
	var pin_i = 0
	var bones = ["Root", "Hips", "Head", "LeftShoulder", "LeftUpperArm", "LeftHand", "RightHand", "LeftFoot", "RightFoot"]
	ewbik.pin_count = bones.size()
	for bone_name in bones:
		var bone_index = skeleton.find_bone(bone_name)
		var node_3d : Node3D = Node3D.new()
		node_3d.name = bone_name
		skeleton.get_parent().add_child(node_3d)
		node_3d.owner = skeleton.owner
		ewbik.set_pin_bone_name(pin_i, bone_name)
		var bone_id = skeleton.find_bone(bone_name)
		if bone_id == -1:
			pin_i = pin_i + 1
			continue
		var bone_global_pose : Transform3D = skeleton.get_bone_global_rest(bone_id)
		bone_global_pose = skeleton.global_pose_to_world_transform(bone_global_pose)
		node_3d.global_transform = bone_global_pose
		ewbik.set_pin_use_node_rotation(pin_i, true)
		if bone_name in ["Root"]:
			ewbik.set_pin_depth_falloff(pin_i, 0)
		var path_string : String = "../../" + str(bone_name)
		ewbik.set_pin_nodepath(pin_i, NodePath(path_string))
		pin_i = pin_i + 1
#	ewbik.constraint_count = profile.bone_size
#	for constraint_i in profile.bone_size:
#		var bone_name : StringName = profile.get_bone_name(constraint_i)
#		if bone_name == null:
#			continue
#		ewbik.set_constraint_name(constraint_i, bone_name)
#		# Female age 9 - 19 https://pubmed.ncbi.nlm.nih.gov/32644411/
#		if bone_name in ["Root"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-0.5))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(0.5))
#		elif bone_name in ["Hips"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-0.5))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(0.5))
#		elif bone_name in ["Spine"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-60))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(60))
#		elif bone_name in ["Chest", "UpperChest"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-30))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(30))
#		elif bone_name in ["Neck"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-47))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(47))
#		elif bone_name in ["Head"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-0.5))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(0.5))
#		elif bone_name in ["LeftShoulder", "RightShoulder"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-18))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(30))
#		elif bone_name in ["LeftUpperArm", "RightUpperArm"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-18))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(30))
#		elif bone_name in ["LeftLowerArm", "RightLowerArm"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-30))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(70))
#			if bone_name == "RightLowerArm":
#				ewbik.set_kusudama_flip_handedness(constraint_i, true)
#			ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
#			ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
#		elif bone_name in ["LeftHand","RightHand"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-40))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(45))
#		elif bone_name in ["LeftUpperLeg", "RightUpperLeg"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-0.5))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(0.5))
#		elif bone_name in ["LeftLowerLeg", "RightLowerLeg"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-0.5))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(0.5))
#		elif bone_name in ["LeftFoot", "RightFoot"]:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-40))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(40))
#		else:
#			ewbik.set_kusudama_twist_from(constraint_i, deg_to_rad(-0.5))
#			ewbik.set_kusudama_twist_to(constraint_i, deg_to_rad(0.5))
		skeleton.notify_property_list_changed()
