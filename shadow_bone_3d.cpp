#include "shadow_bone_3d.h"
#include "kusudama_constraint.h"
#include "core/object/reference.h"

void ShadowBone3D::set_constraint(Ref<KusudamaConstraint> p_constraint) {
	constraint = p_constraint;
}
Ref<KusudamaConstraint> ShadowBone3D::get_constraint() const {
	return constraint;
}
void ShadowBone3D::update_cos_dampening(int p_default_iterations, float p_default_dampening) {
	float predampening = 1.0f - sim_local_ik_node.stiffness;
	float dampening = sim_constraint_ik_node.parent == -1 ? Math_PI : predampening * p_default_dampening;
	cos_half_dampen = Math::cos(dampening / 2.0f);
	if (constraint.is_valid() && constraint->get_pain() != 0.0f) {
		springy = true;
		populate_return_dampening_iteration_array(p_default_iterations, p_default_dampening);
	} else {
		springy = false;
	}
}
void ShadowBone3D::populate_return_dampening_iteration_array(int p_default_iterations, float p_default_dampening) {
	float predampen = 1.0f - sim_constraint_ik_node.stiffness;
	float dampening = sim_constraint_ik_node.parent == -1 ? Math_PI : predampen * p_default_dampening;
	float returnful = constraint->get_pain();
	float falloff = 0.2f;
	half_returnful_dampened.resize(p_default_iterations);
	cos_half_returnful_dampened.resize(p_default_iterations);
	float iterations_pow = Math::pow(p_default_iterations, falloff * p_default_iterations * returnful);
	for (int32_t iter_i = 0; iter_i < p_default_iterations; iter_i++) {
		float iteration_scalar =
				((iterations_pow)-Math::pow(iter_i, falloff * p_default_iterations * returnful)) / (iterations_pow);
		float iteration_return_clamp = iteration_scalar * returnful * dampening;
		float cos_iteration_return_clamp = Math::cos(iteration_return_clamp / 2.0f);
		half_returnful_dampened.write[iter_i] = iteration_return_clamp;
		cos_half_returnful_dampened.write[iter_i] = cos_iteration_return_clamp;
	}
}