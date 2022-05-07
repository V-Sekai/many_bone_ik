#pragma once

#include "core/object/ref_counted.h"

class IKEffectorTemplate : public Resource {
	GDCLASS(IKEffectorTemplate, Resource);

	NodePath target_node;
	float depth_falloff = 1.0f;
	bool is_target_node_rotation = true;

protected:
	static void _bind_methods();

public:
	NodePath get_target_node() const;
	void set_target_node(NodePath p_node_path);
	float get_depth_falloff() const;
	void set_depth_falloff(float p_depth_falloff);
	bool get_target_node_rotation() const;
	void set_target_node_rotation(bool p_target_node_rotation);
	IKEffectorTemplate();
};