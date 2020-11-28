#pragma once

#include "core/config/engine.h"
#include "core/object/reference.h"
#include "ray.h"

class DMIKAxisDependency : public Reference {
public:
	virtual void emancipate(){};
	virtual void axis_slip_warning(Ref<DMIKAxisDependency> p_global_prior_to_slipping, Ref<DMIKAxisDependency> p_global_after_slipping, Ref<DMIKAxisDependency> p_this_axis){};
	virtual void axis_slip_completion_notice(Ref<DMIKAxisDependency> p_global_prior_to_slipping, Ref<DMIKAxisDependency> p_global_after_slipping, Ref<DMIKAxisDependency> p_this_axis){};
	virtual void parent_change_warning(Ref<DMIKAxisDependency> p_warning_by, Ref<DMIKAxisDependency> p_old_parent, Ref<DMIKAxisDependency> p_intended_parent, Object p_requested_by){};
	virtual void parent_change_completion_notice(Ref<DMIKAxisDependency> p_warning_by, Ref<DMIKAxisDependency> p_old_parent, Ref<DMIKAxisDependency> p_intended_parent, Object p_requested_by){};
	virtual void mark_dirty(){};
	virtual void mark_dependents_dirty(){};
	virtual Ref<DMIKAxisDependency> get_parent_axes() = 0;
};
