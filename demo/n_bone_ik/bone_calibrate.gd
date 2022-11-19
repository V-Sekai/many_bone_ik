@tool
extends EditorScript

var human_bones: PackedStringArray

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
			for constraint_i in range(human_bones.size()):
				var bone_name = human_bones[constraint_i]
				ik.set_constraint_name(constraint_i, bone_name)
				var twist_current : float = ik.get_kusudama_twist_current(constraint_i)
				print("%s %s" % [bone_name, twist_current])
				ik.set_kusudama_twist(constraint_i, Vector2(twist_current * (TAU - 0.001), TAU - 0.01))
			ik.queue_print_skeleton()
			break
