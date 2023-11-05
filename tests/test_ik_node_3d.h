#ifndef TEST_IKNODE3D_H
#define TEST_IKNODE3D_H

#include "tests/test_macros.h"
#include "modules/many_bone_ik/src/math/ik_node_3d.h"

namespace TestIKNode3D {

TEST_CASE("[Modules][IKNode3D] Transform operations") {
    Ref<IKNode3D> node = memnew(IKNode3D);

    // Test set_transform and get_transform
    Transform3D t;
    t.origin = Vector3(1, 2, 3);
    node->set_transform(t);
    CHECK(node->get_transform() == t);

    // Test set_global_transform and get_global_transform
    Transform3D gt;
    gt.origin = Vector3(4, 5, 6);
    node->set_global_transform(gt);
    CHECK(node->get_global_transform() == gt);
}

TEST_CASE("[Modules][IKNode3D] Scale operations") {
    Ref<IKNode3D> node = memnew(IKNode3D);

    // Test set_disable_scale and is_scale_disabled
    node->set_disable_scale(true);
    CHECK(node->is_scale_disabled() == true);
}

TEST_CASE("[Modules][IKNode3D] Parent operations") {
    Ref<IKNode3D> node = memnew(IKNode3D);
    Ref<IKNode3D> parent = memnew(IKNode3D);

    // Test set_parent and get_parent
    node->set_parent(parent);
    CHECK(node->get_parent() == parent);
}

TEST_CASE("[Modules][IKNode3D] Coordinate transformations") {
    Ref<IKNode3D> node = memnew(IKNode3D);

    // Test to_local and to_global
    Vector3 global(1, 2, 3);
    Vector3 local = node->to_local(global);
    CHECK(node->to_global(local) == global);
}

TEST_CASE("[Modules][IKNode3D] Test local transform calculation") {
    Ref<IKNode3D> node = memnew(IKNode3D);

    Transform3D node_transform;
    node_transform.origin = Vector3(1.0, 2.0, 3.0); // Translation by (1, 2, 3)
    node->set_global_transform(node_transform);

    Ref<IKNode3D> parent_node = memnew(IKNode3D);

    Transform3D parent_transform;
    parent_transform.origin = Vector3(4.0, 5.0, 6.0); // Translation by (4, 5, 6)
    parent_node->set_global_transform(parent_transform);

    node->set_parent(parent_node);

    Transform3D expected_local_transform = parent_node->get_global_transform().affine_inverse() * node->get_global_transform();

    CHECK(node->get_transform() == expected_local_transform);
}
} // namespace TestIKNode3D

#endif // TEST_IKNODE3D_H
