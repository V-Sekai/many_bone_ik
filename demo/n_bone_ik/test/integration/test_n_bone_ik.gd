extends GutTest

func test_assert_eq_integration_n_bone_instantiation_not_null():
	assert_not_null(NBoneIK.new(), "Can instantiate NBoneIK")
	
func test_assert_eq_integration_n_bone_create_skeleton():
	assert_not_null(Skeleton3D.new(), "Can instantiate Skeleton3D")
	