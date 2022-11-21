@tool
extends EditorScript

func enable_debug_pins(pins, new_ik : NBoneIK) -> void:
	for pin_i in range(pins.size()):
		new_ik.set_pin_weight(pin_i, 0)
		if str(pins[pin_i]) == "Root":
			new_ik.set_pin_weight(pin_i, 1)
		new_ik.default_damp = 0.001

<<<<<<< Updated upstream
=======
var bones: PackedStringArray

func constraint(new_ik : NBoneIK, skeleton : Skeleton3D, root : Node) -> void:	
	new_ik.set_constraint_count(bones.size())
	for constraint_i in range(bones.size()):
		var bone_name = bones[constraint_i]
		new_ik.set_constraint_name(constraint_i, bone_name)
	var pins : Array
	for bone_i in skeleton.get_bone_count():
		var bone_name = skeleton.get_bone_name(bone_i)
		if bone_i == -1:
			continue
		if pins.has(bone_name):
			continue
		if bone_name.ends_with("_end"):
			continue
		if bone_name.find("HairJoint") != -1:
			continue
		if bone_name.find("_TipSleeve") != -1:
			continue
		if bone_name.find("_LowerSleeve") != -1:
			continue
		if bone_name.find("_Bust") != -1:
			continue
		if bone_name.find("_SkirtSide") != -1:
			continue
		if bone_name.find("_SkirtBack") != -1:
			continue
		if bone_name.find("_SkirtFront") != -1:
			continue
		if bone_name.find("_SkirtFront") != -1:
			continue
		if bone_name.find("LeftEye") != -1:
			continue
		if bone_name.find("RightEye") != -1:
			continue
		pins.push_back(bone_name)
	for bone_i in skeleton.get_bone_count():
		var bone_name = skeleton.get_bone_name(bone_i)
		var node = root.find_child(bone_name)
		if node != null:
			node.free()
	skeleton.reset_bone_poses()
	new_ik.set_constraint_count(0)
	new_ik.set_pin_count(0)
#	for constraint_i in skeleton.get_bone_count():
#		var bone_name = skeleton.get_bone_name(constraint_i)
#		var twist_min = new_ik.get_kusudama_twist(constraint_i).x
#		if bone_name in ["UpperChest"]:
#			new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, PI))
#		elif bone_name in ["Chest"]:
#			new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, PI))
#		elif bone_name in ["Spine"]:
#			new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, PI))
#		elif bone_name.ends_with("UpperArm"):
#			new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, PI))
#		elif bone_name.ends_with("LowerArm"):
#			new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, PI))
#
#	for constraint_i in skeleton.get_bone_count():
#		var bone_name : String = new_ik.get_constraint_name(constraint_i)
#		if bone_name in ["Head"]:
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#		elif bone_name in ["Neck"]:
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
#		elif bone_name in ["UpperChest"]:
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
#		elif bone_name in ["Chest"]:
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
#		elif bone_name in ["Spine"]:
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
#		elif bone_name in ["Hips"]:
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
#		elif bone_name.ends_with("Shoulder"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(-1, 0, 0))
#			if bone_name.begins_with("Left"):
#				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(1, 0, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(30))
#		elif bone_name.ends_with("UpperArm"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
#		elif bone_name.ends_with("LowerArm"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 3)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(1, 0, 0))
#			if bone_name.begins_with("Left"):
#				new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(-1, 0, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(1))
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(1))
#		elif bone_name.ends_with("Hand"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
#		elif bone_name.ends_with("UpperLeg"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
#		elif bone_name.ends_with("LowerLeg"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 3)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(1))
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(1))
#		elif bone_name.ends_with("Foot"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 2)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
#		elif bone_name.ends_with("Toes"):
#			new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
#			new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
#			new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))

	new_ik.set_pin_count(pins.size())
	for pin_i in range(pins.size()):
		var pin = pins[pin_i]
		var bone_name = pin
		var bone_i = skeleton.find_bone(bone_name)
		if bone_i == -1:
			continue
		if bone_name in ["Hips"]:
			new_ik.set_pin_passthrough_factor(pin_i, 0.1)
		new_ik.set_pin_bone_name(pin_i, bone_name)
		new_ik.set_pin_passthrough_factor(pin_i, 1)			
		if bone_name in ["UpperChest"]:
			new_ik.set_pin_weight(pin_i, 0.01)
		var marker_3d : Marker3D = Marker3D.new()
		marker_3d.name = bone_name
		marker_3d.global_transform = skeleton.get_bone_global_rest(skeleton.find_bone(bone_name))
		skeleton.add_child(marker_3d)
		var path_string : String = "../" + str(skeleton.get_path_to(marker_3d))
		new_ik.set_pin_nodepath(pin_i, NodePath(path_string))
		marker_3d.owner = root
		marker_3d.gizmo_extents = 0.1
		
>>>>>>> Stashed changes
func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	for skeleton in skeletons:
		if not skeleton.get_bone_count():
			continue
		var iks : Array[Node] = skeleton.find_children("*", "NBoneIK")
		for ik in iks:
			ik.free()
		var new_ik : NBoneIK = NBoneIK.new()
		skeleton.add_child(new_ik, true)
		new_ik.owner = root
		new_ik.visible = false
		new_ik.set_pin_count(0)
		new_ik.set_constraint_count(0)
		var pins =  [
			skeleton.get_bone_name(skeleton.get_parentless_bones()[0])
		]
		for bone_i in skeleton.get_bone_count():
			var bone_name = skeleton.get_bone_name(bone_i)
			if bone_i == -1:
				continue
			if pins.has(bone_name):
				continue
			pins.push_back(bone_name)
		for pin in pins:
			var node = root.find_child(pin)
			if node != null:
				node.free()
		skeleton.reset_bone_poses()
		new_ik.set_pin_count(pins.size())
		
		for pin_i in range(pins.size()):
			var pin = pins[pin_i]
			var bone_name = pin
			var bone_i = skeleton.find_bone(bone_name)
			if bone_i == -1:
				continue
			if bone_name in ["Hips"]:
				new_ik.set_pin_passthrough_factor(pin_i, 0.1)
			var node_3d : BoneAttachment3D = BoneAttachment3D.new()
			node_3d.name = bone_name
			node_3d.bone_name = bone_name
			node_3d.bone_idx = skeleton.find_bone(bone_name)
			skeleton.add_child(node_3d)
			node_3d.owner = root
			var path_string : String = "../" + str(skeleton.get_path_to(node_3d))
			new_ik.set_pin_nodepath(pin_i, NodePath(path_string))
			new_ik.set_pin_bone_name(pin_i, bone_name)
			new_ik.set_pin_passthrough_factor(pin_i, 1)			
			if bone_name in ["UpperChest"]:
				new_ik.set_pin_weight(pin_i, 0.01)
			var marker_3d : Marker3D = Marker3D.new()
			marker_3d.name = bone_name
			marker_3d.global_transform = node_3d.global_transform
			node_3d.replace_by(marker_3d, true)
			marker_3d.gizmo_extents = 0.1

		new_ik.visible = true
