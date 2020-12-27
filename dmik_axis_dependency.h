#pragma once

#include "core/config/engine.h"
#include "core/object/reference.h"
#include "ray.h"

class EWBIKAxisDependency : public Reference {
public:
	virtual void emancipate(){};
	virtual void axis_slip_warning(Ref<EWBIKAxisDependency> p_global_prior_to_slipping, Ref<EWBIKAxisDependency> p_global_after_slipping, Ref<EWBIKAxisDependency> p_this_axis){};
	virtual void axis_slip_completion_notice(Ref<EWBIKAxisDependency> p_global_prior_to_slipping, Ref<EWBIKAxisDependency> p_global_after_slipping, Ref<EWBIKAxisDependency> p_this_axis){};
	virtual void parent_change_warning(Ref<EWBIKAxisDependency> p_warning_by, Ref<EWBIKAxisDependency> p_old_parent, Ref<EWBIKAxisDependency> p_intended_parent, Object p_requested_by){};
	virtual void parent_change_completion_notice(Ref<EWBIKAxisDependency> p_warning_by, Ref<EWBIKAxisDependency> p_old_parent, Ref<EWBIKAxisDependency> p_intended_parent, Object p_requested_by){};
	virtual void mark_dirty(){};
	virtual void mark_dependents_dirty(){};
	virtual Ref<EWBIKAxisDependency> get_parent_axes() = 0;
};
