# This script defines a RotationOrder class for handling different rotation orders in 3D space.
@tool
extends Object

class_name RotationOrder

# Define the unit vectors for each axis.
const X = Vector3(1, 0, 0)
const Y = Vector3(0, 1, 0)
const Z = Vector3(0, 0, 1)

## Define the different rotation orders as constants.
#const XYZ = RotationOrder.new("XYZ", X, Y, Z)
#const XZY = RotationOrder.new("XZY", X, Z, Y)
#const YXZ = RotationOrder.new("YXZ", Y, X, Z)
#const YZX = RotationOrder.new("YZX", Y, Z, X)
#const ZXY = RotationOrder.new("ZXY", Z, X, Y)
#const ZYX = RotationOrder.new("ZYX", Z, Y, X)
#const XYX = RotationOrder.new("XYX", X, Y, X)
#const XZX = RotationOrder.new("XZX", X, Z, X)
#const YXY = RotationOrder.new("YXY", Y, X, Y)
#const YZY = RotationOrder.new("YZY", Y, Z, Y)
#const ZXZ = RotationOrder.new("ZXZ", Z, X, Z)
#const ZYZ = RotationOrder.new("ZYZ", Z, Y, Z)

# Instance variables to store the name and axes of the rotation order.
var name: String
var a_1: Vector3
var a_2: Vector3
var a_3: Vector3

# Initialize the RotationOrder instance with the given name and axes.
func _init(p_name: String, p_a_1: Vector3, p_a_2: Vector3, p_a_3: Vector3) -> void:
	name = p_name
	a_1 = p_a_1
	a_2 = p_a_2
	a_3 = p_a_3

# Return the name of the rotation order.
func print() -> String:
	return name

# Getters for the axes of the rotation order.
func get_a_1() -> Vector3:
	return a_1

func get_a_2() -> Vector3:
	return a_2

func get_a_3() -> Vector3:
	return a_3
