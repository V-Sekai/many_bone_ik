#include "core/resource.h"

class BoneEffector : public Resource {
	GDCLASS(BoneEffector, Resource);

protected:
	Transform target_transform;
	NodePath target_node;
	real_t budget_ms = 4.0f;
	static void _bind_methods();

public:
	void set_target_transform(Transform p_target_transform);
	Transform get_target_transform() const;
	void set_target_node(NodePath p_target_node_path);
	NodePath get_target_node() const;
	real_t get_budget_ms() const;
	void set_budget_ms(real_t p_budget_ms);
};
