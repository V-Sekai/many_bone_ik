@tool
extends EditorScript

func enable_debug_pins(pins, new_ik : NBoneIK) -> void:
	for pin_i in range(pins.size()):
		new_ik.set_pin_weight(pin_i, 0)
		if str(pins[pin_i]) == "Root":
			new_ik.set_pin_weight(pin_i, 1)
		new_ik.default_damp = 0.001

func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	for skeleton in skeletons:
		if not skeleton.get_bone_count():
			continue
		var iks : Array[Node] = skeleton.find_children("*", "NBoneIK")
		var edit_mode = false
		for ik in iks:
			edit_mode = ik.edit_constraints
			ik.free()
		var new_ik : NBoneIK = NBoneIK.new()
		skeleton.add_child(new_ik, true)
		new_ik.owner = root
		new_ik.visible = false
		new_ik.set_pin_count(0)
		new_ik.set_constraint_count(0)
		var pins : Array
		for bone_i in skeleton.get_bone_count():
			var bone_name = skeleton.get_bone_name(bone_i)
			if bone_i == -1:
				continue
			if pins.has(bone_name):
				continue
			if bone_name.find("_HoodString") != -1:
				continue
			if bone_name.find("HairJoint-") != -1:
				continue
			if bone_name.ends_with("C_Hood"):
				continue
			if bone_name.ends_with("_end"):
				continue
			if bone_name == "RightEye":
				continue
			if bone_name == "LeftEye":
				continue
			if bone_name.ends_with("UpperArm"):
				continue
			if bone_name.ends_with("LowerArm"):
				continue
			if bone_name.ends_with("Proximal"):
				continue
			if bone_name.ends_with("Proximal2"):
				continue
			if bone_name.ends_with("Metacarpal"):
				continue
			if bone_name.ends_with("Metacarpal2"):
				continue
			if bone_name.ends_with("Distal"):
				continue
			if bone_name.ends_with("Intermediate"):
				continue
			if bone_name.ends_with("Toes"):
				continue
			if bone_name.ends_with("UpperChest"):
				continue
			if bone_name.ends_with("UpperLeg"):
				continue
			if bone_name.ends_with("LowerLeg"):
				continue
			if bone_name.ends_with("Chest"):
				continue
			if bone_name.ends_with("Spine"):
				continue
			if bone_name.ends_with("Neck"):
				continue
			if bone_name.ends_with("Shoulder"):
				continue
			if bone_name.ends_with("RightHand"): # TODO: Remove after testing.
				continue
			if bone_name.ends_with("Foot"): # TODO: Remove after testing.
				continue
			pins.push_back(bone_name)
		for pin in pins:
			var node = root.find_child(pin)
			if node != null:
				node.free()
		skeleton.reset_bone_poses()
		new_ik.set_pin_count(pins.size())
		
		var constraints : Array
		for bone_i in skeleton.get_bone_count():
			var bone_name = skeleton.get_bone_name(bone_i)
			if bone_i == -1:
				continue
			if bone_name.find("_HoodString") != -1:
				continue
			if bone_name.find("HairJoint-") != -1:
				continue
			if bone_name.ends_with("C_Hood"):
				continue
			if bone_name.ends_with("_end"):
				continue
			if bone_name == "RightEye":
				continue
			if bone_name == "LeftEye":
				continue
			if bone_name.ends_with("Proximal"):
				continue
			if bone_name.ends_with("Proximal2"):
				continue
			if bone_name.ends_with("Metacarpal"):
				continue
			if bone_name.ends_with("Metacarpal2"):
				continue
			if bone_name.ends_with("Distal"):
				continue
			if bone_name.ends_with("Intermediate"):
				continue
			if bone_name.ends_with("Toes"):
				continue
			constraints.push_back(bone_name)
		new_ik.set_constraint_count(constraints.size())
		for pin_i in pins.size():
			var pin = pins[pin_i]
			var bone_name = pin
			var bone_i = skeleton.find_bone(bone_name)
			if bone_i == -1:
				continue
			if bone_name in ["Hips"]:
				new_ik.set_pin_passthrough_factor(pin_i, 0.1)
			new_ik.set_pin_bone_name(pin_i, bone_name)
			new_ik.set_pin_passthrough_factor(pin_i, 0.8)
			if bone_name in ["Root"]:
				new_ik.set_pin_passthrough_factor(pin_i, 0)
				new_ik.set_pin_weight(pin_i, 0.3)
				new_ik.set_pin_direction_priorities(pin_i, Vector3(0,0,0))
			if bone_name in ["Hips"]:
				new_ik.set_pin_passthrough_factor(pin_i, 0)
				new_ik.set_pin_weight(pin_i, 0.3)
				new_ik.set_pin_direction_priorities(pin_i, Vector3(0,0,0))
			if bone_name in ["UpperChest"]:
				new_ik.set_pin_weight(pin_i, 0.01)
			
		for constraint_i in constraints.size():
			var bone_name = constraints[constraint_i]
			new_ik.set_constraint_name(constraint_i, bone_name)

		for constraint_i in new_ik.get_constraint_count():
			var bone_name : String = new_ik.get_constraint_name(constraint_i)
			var twist_min = new_ik.get_kusudama_twist(constraint_i).x
			if bone_name.ends_with("Root"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(371.3), deg_to_rad(33.4)))
			elif bone_name.ends_with("Hips"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(340.1), deg_to_rad(25)))
			elif bone_name.ends_with("Spine"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(355), deg_to_rad(30)))
			elif bone_name.ends_with("Chest"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(355), deg_to_rad(30)))
			elif bone_name.ends_with("UpperChest"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(355), deg_to_rad(30)))
			# HEAD ---------
			elif bone_name.ends_with("Head"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(5)))
			elif bone_name.ends_with("Neck"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(356), deg_to_rad(5)))
			# ARMS ---------
			elif bone_name.ends_with("RightUpperArm"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(95), deg_to_rad(20)))
			elif bone_name.ends_with("LeftUpperArm"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(95), deg_to_rad(20)))
			elif bone_name.ends_with("RightLowerArm"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(110), deg_to_rad(5)))
			elif bone_name.ends_with("LeftLowerArm"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(250), deg_to_rad(5)))
			elif bone_name.ends_with("RightShoulder"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(5)))
			elif bone_name.ends_with("LeftShoulder"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(295.1), deg_to_rad(5)))
			elif bone_name.ends_with("RightHand"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(78.6), deg_to_rad(5)))
			elif bone_name.ends_with("LeftHand"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(5)))
			# LEGS ---------
			elif bone_name.ends_with("RightUpperLeg"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(19.2), deg_to_rad(30.6)))
			elif bone_name.ends_with("LeftUpperLeg"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(207.6), deg_to_rad(30)))
			elif bone_name.ends_with("LowerLeg"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(5)))
			elif bone_name.ends_with("Foot"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
			elif bone_name.ends_with("Toes"):
				new_ik.set_kusudama_twist(constraint_i, Vector2(twist_min, deg_to_rad(30)))
		for bone_i in constraints.size():
			var bone_name : String =  constraints[bone_i]
			var constraint_i = new_ik.find_constraint(bone_name)
			if bone_name in ["Head"]:
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
			elif bone_name in ["Hips"]:
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
			elif bone_name in ["Neck"]:
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			elif bone_name in ["UpperChest"]:
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			elif bone_name in ["Chest"]:
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(5))
			elif bone_name in ["Spine"]:
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
			elif bone_name.ends_with("Shoulder"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(-1, 0, 0))
				if bone_name.begins_with("Left"):
					new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(1, 0, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			elif bone_name.ends_with("UpperArm"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
			elif bone_name.ends_with("LowerArm"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 3)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
				new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(1, 0, 0))
				if bone_name.begins_with("Left"):
					new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(-1, 0, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(20))
				new_ik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(10))
			elif bone_name.ends_with("Hand"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(20))
			elif bone_name.ends_with("UpperLeg"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(50))
			elif bone_name.ends_with("LowerLeg"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 3)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(1))
				new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(1))
				new_ik.set_kusudama_limit_cone_center(constraint_i, 2, Vector3(0, -1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 2, deg_to_rad(1))
			elif bone_name.ends_with("Foot"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 2)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, -1, 0))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(10))
				new_ik.set_kusudama_limit_cone_center(constraint_i, 1, Vector3(0, 0, -1))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 1, deg_to_rad(10))
			elif bone_name.ends_with("Toes"):
				new_ik.set_kusudama_limit_cone_count(constraint_i, 1)
				new_ik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 0, -1))
				new_ik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(15))
				
		for bone_i in new_ik.get_pin_count():
			var bone_name : String = new_ik.get_pin_bone_name(bone_i)
			var node_3d : BoneAttachment3D = BoneAttachment3D.new()
			node_3d.name = bone_name
			node_3d.bone_name = bone_name
			node_3d.bone_idx = skeleton.find_bone(bone_name)
			skeleton.add_child(node_3d)
			node_3d.owner = root
			var node_global_transform = node_3d.global_transform
			var path_string : String = "../" + str(skeleton.get_path_to(node_3d))
			new_ik.set_pin_nodepath(bone_i, NodePath(path_string))
			var marker_3d : Marker3D = Marker3D.new()
			marker_3d.name = bone_name
			marker_3d.global_transform = node_global_transform
			node_3d.replace_by(marker_3d, true)
		new_ik.visible = true
		new_ik.edit_constraints = edit_mode
