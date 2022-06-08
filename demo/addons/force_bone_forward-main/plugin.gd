@tool
extends EditorPlugin

var import_plugin : EditorScenePostImportPlugin = null
var correct_bone_dir_script = load("res://addons/force_bone_forward/correct_bone_dir.gd")

func _enter_tree():
	import_plugin = correct_bone_dir_script.new()
	add_scene_post_import_plugin(import_plugin)


func _exit_tree():
	add_scene_post_import_plugin(import_plugin)
	import_plugin = null
