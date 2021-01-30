#include "ewbik_task.h"
#include "core/object/reference.h"

EWBIKTask::EWBIKTask(Ref<SkeletonModification3DEWBIK> p_ewbik) {
	Ref<EWBIKSegmentedSkeleton3D> new_chain = memnew(EWBIKSegmentedSkeleton3D(p_ewbik));
	chain = new_chain;
	ewbik = p_ewbik;
}