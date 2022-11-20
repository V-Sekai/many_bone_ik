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
			var new_ik : NBoneIK = ik
			for i in range(5):
				for constraint_i in range(human_bones.size()):
					var bone_name = human_bones[constraint_i]
					var twist_current : float = new_ik.get_kusudama_twist_current(constraint_i)
					var twist_min : float = new_ik.get_kusudama_twist(constraint_i).x
					var twist_range : float = new_ik.get_kusudama_twist(constraint_i).y
					if twist_range < deg_to_rad(10):
						continue
					var current = new_ik.get_kusudama_twist_current(constraint_i) 
					new_ik.set_kusudama_twist(constraint_i, Vector2(lerp_angle(0, TAU - twist_min, current), lerp_angle(0, twist_range, current)))
					if current > 0.1:
						new_ik.set_kusudama_twist(constraint_i, Vector2(lerp_angle(0, TAU - twist_min, 0.5), lerp_angle(0, twist_range, 0.5)))
