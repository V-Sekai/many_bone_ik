#include "ewbik_node_3d.h"

Ref<EWBIKNode3D> EWBIKNode3D::relativeTo(Ref<EWBIKNode3D> in) {
	return Ref<EWBIKNode3D>();
}

DMIKTransform EWBIKNode3D::getLocalOf(EWBIKNode3D input) {
	return DMIKTransform();
}

Ref<EWBIKNode3D> EWBIKNode3D::getLocalOf(Ref<EWBIKNode3D> input) {
	return Ref<EWBIKNode3D>();
}

Ref<EWBIKNode3D> EWBIKNode3D::freeCopy() {
	return Ref<EWBIKNode3D>();
}

Ray EWBIKNode3D::x_() {
	return Ray();
}

Ray EWBIKNode3D::y_() {
	return Ray();
}

Ray EWBIKNode3D::z_() {
	return Ray();
}

Ref<EWBIKNode3D> EWBIKNode3D::attachedCopy(bool slipAware) {
	return Ref<EWBIKNode3D>();
}

void EWBIKNode3D::mark_dirty() {
	if (!dirty) {
		dirty = true;
		mark_dependents_dirty();
	}
}

void EWBIKNode3D::mark_dependents_dirty() {
	List<Ref<EWBIKAxisDependency>>::Element *i = dependentsRegistry.front();
	while (i) {
		Ref<EWBIKAxisDependency> dr = i->get();
		if (dr.is_valid()) {
			dr->mark_dirty();
		}
		i = i->next();
	}
}

EWBIKNode3D::EWBIKNode3D(Transform p_globalBasis, Ref<EWBIKNode3D> p_parent) {
	globalMBasis = p_globalBasis;
	createTempVars(p_globalBasis.origin);
	if (get_parent_axes().is_valid()) {
		setParent(p_parent);
	} else {
		areGlobal = true;
		localMBasis = globalMBasis;
	}
	updateGlobal();
}

void EWBIKNode3D::updateGlobal() {
	if (dirty) {
		if (areGlobal) {
			globalMBasis.adopt_values(localMBasis);
		} else {
			Ref<EWBIKNode3D> parent_axes = get_parent_axes();
			if (parent_axes.is_null()) {
				return;
			}
			parent_axes->updateGlobal();
			parent_axes->getGlobalMBasis().set_to_global_of(localMBasis, globalMBasis);
		}
	}
	dirty = false;
}
void EWBIKNode3D::setToLocalOf(DMIKTransform input, DMIKTransform &output) {
	updateGlobal();
	getGlobalMBasis().set_to_local_of(input, output);
}
void EWBIKNode3D::setToLocalOf(Vector3 in, Vector3 &out) {
	updateGlobal();
	getGlobalMBasis().set_to_local_of(in, out);
}

void EWBIKNode3D::setToLocalOf(Ray in, Ray &out) {
	setToLocalOf(in.position, out.position);
	setToLocalOf(in.normal, out.normal);
}

Vector3 EWBIKNode3D::setToLocalOf(Vector3 in) {
	updateGlobal();
	Vector3 result;
	getGlobalMBasis().set_to_local_of(in, result);
	return result;
}

void EWBIKNode3D::setToGlobalOf(Ray input, Ray &output) {
	updateGlobal();
	setToGlobalOf(input.position, output.position);
	setToGlobalOf(input.normal, output.normal);
}

void EWBIKNode3D::setToGlobalOf(Vector3 input, Vector3 &output) {
	updateGlobal();
	getGlobalMBasis().set_to_global_of(input, output);
}

Vector3 EWBIKNode3D::setToGlobalOf(Vector3 in) {
	updateGlobal();
	getGlobalMBasis().set_to_global_of(in, in);
	return in;
}

void EWBIKNode3D::createTempVars(Vector3 type) {
	workingVector = type;
	tempOrigin = type;
}

Ref<EWBIKAxisDependency> EWBIKNode3D::get_parent_axes() {
	if (parent.is_null()) {
		return nullptr;
	} else {
		return parent;
	}
}

void EWBIKNode3D::setParent(Ref<EWBIKNode3D> par) {
	setParent(par, nullptr);
}

void EWBIKNode3D::setParent(Ref<EWBIKNode3D> intendedParent, Ref<EWBIKNode3D> requestedBy) {
	updateGlobal();
	Ref<EWBIKNode3D> oldParent = get_parent_axes();
	// forEachDependent(
	// 		(ad)->ad.get().parentChangeWarning(this, oldParent, intendedParent, requestedBy));

	if (intendedParent != nullptr && intendedParent != this) {
		intendedParent->updateGlobal();
		intendedParent->getGlobalMBasis().set_to_local_of(globalMBasis, localMBasis);

		if (oldParent != nullptr) {
			oldParent->disown(this);
		}
		parent = intendedParent;

		Ref<EWBIKNode3D> axes = get_parent_axes();
		if (axes.is_valid()) {
			axes->registerDependent(this);
		}
		areGlobal = false;
	} else {
		if (oldParent != nullptr)
			oldParent->disown(this);
		parent.unref();
		areGlobal = true;
	}
	mark_dirty();
	updateGlobal();
	// forEachDependent(
	// 		(ad)->ad.get().parentChangeCompletionNotice(this, oldParent, intendedParent, requestedBy));
}

EWBIKNode3D::EWBIKNode3D(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, Ref<EWBIKNode3D> parent, bool customBases) {
	if (!customBases) {
		globalMBasis = parent.is_valid() ? parent->getGlobalMBasis() : DMIKTransform(origin);
		localMBasis = parent.is_valid() ? parent->getLocalMBasis() : DMIKTransform(origin);
		globalMBasis.set_identity();
		localMBasis.set_identity();
	}
	if (parent.is_null()) {
		areGlobal = true;
	}
}
EWBIKNode3D::EWBIKNode3D() {
	areGlobal = true;
	updateGlobal();
}

EWBIKNode3D::EWBIKNode3D(DMIKTransform p_globalBasis, Ref<EWBIKNode3D> p_parent) {
	globalMBasis = p_globalBasis;
	createTempVars(p_globalBasis.get_origin());
	if (get_parent_axes().is_valid()) {
		setParent(p_parent);
	} else {
		areGlobal = true;
		localMBasis = p_globalBasis;
	}
	updateGlobal();
}

Vector3 EWBIKNode3D::origin_() {
	updateGlobal();
	tempOrigin = getGlobalMBasis().get_origin();
	return tempOrigin;
}

Ref<EWBIKNode3D> EWBIKNode3D::getGlobalCopy() {
	Ref<EWBIKNode3D> node;
	node.instance();
	DMIKTransform global_basis = this->globalMBasis;
	node->globalMBasis = global_basis;
	createTempVars(global_basis.get_origin());
	Ref<EWBIKNode3D> parent_axes = this->get_parent_axes();
	if (parent_axes.is_valid()) {
		setParent(parent_axes);
	} else {
		areGlobal = true;
		localMBasis = this->globalMBasis;
	}
	updateGlobal();
	return node;
}

int EWBIKNode3D::getGlobalChirality() {
	updateGlobal();
	return getGlobalMBasis().chirality;
}

int EWBIKNode3D::getLocalChirality() {
	updateGlobal();
	return getLocalMBasis().chirality;
}

bool EWBIKNode3D::isGlobalAxisFlipped(int axis) {
	updateGlobal();
	return globalMBasis.is_axis_flipped(axis);
}

bool EWBIKNode3D::isLocalAxisFlipped(int axis) {
	return localMBasis.is_axis_flipped(axis);
}

void EWBIKNode3D::setRelativeToParent(Ref<EWBIKNode3D> par) {
	Ref<EWBIKNode3D> axes = get_parent_axes();
	if (axes.is_valid()) {
		axes->disown(this);
	}
	parent = par;
	axes = get_parent_axes();
	if (axes.is_valid()) {
		axes->registerDependent(this);
	}
	mark_dirty();
}

bool EWBIKNode3D::needsUpdate() {
	if (dirty)
		return true;
	else
		return false;
}

Ray EWBIKNode3D::getGlobalOf(Ray in) {
	return Ray(getGlobalOf(in.position), getGlobalOf(in.normal));
}

Vector3 EWBIKNode3D::getGlobalOf(Vector3 in) {
	Vector3 result = in;
	setToGlobalOf(in, result);
	return result;
}

Ray EWBIKNode3D::getLocalOf(Ray in) {
	Ray result = in;
	result.position = getLocalOf(in.position);
	result.normal = getLocalOf(in.normal);
	return result;
}

Vector3 EWBIKNode3D::getLocalOf(Vector3 in) {
	updateGlobal();
	return getGlobalMBasis().get_local_of(in);
}

void EWBIKNode3D::translateByLocal(Vector3 translate) {
	updateGlobal();
	getLocalMBasis().translate_by(translate);
	mark_dirty();
}

void EWBIKNode3D::translateByGlobal(Vector3 translate) {
	if (get_parent_axes().is_valid()) {
		updateGlobal();
		translateTo(translate + origin_());
	} else {
		getLocalMBasis().translate_by(translate);
	}

	mark_dirty();
}

void EWBIKNode3D::translateTo(Vector3 translate) {
	Ref<EWBIKNode3D> parent_axes = get_parent_axes();
	if (parent_axes.is_valid()) {
		updateGlobal();
		getLocalMBasis().translate_to(parent_axes->getGlobalMBasis().get_local_of(translate));
		mark_dirty();
	} else {
		updateGlobal();
		getLocalMBasis().translate_to(translate);
		mark_dirty();
	}
}

void EWBIKNode3D::translateTo(Vector3 translate, bool slip) {
	updateGlobal();
	if (slip) {
		Ref<EWBIKNode3D> tempAbstractAxes = getGlobalCopy();
		tempAbstractAxes->translateTo(translate);
		slipTo(tempAbstractAxes);
	} else {
		translateTo(translate);
	}
}

void EWBIKNode3D::setSlipType(int type) {
	Ref<EWBIKNode3D> parent_axes = get_parent_axes();
	if (parent_axes.is_valid()) {
		if (type == IGNORE) {
			parent_axes->dependentsRegistry.erase(this);
		} else if (type == NORMAL || type == FORWARD) {
			parent_axes->registerDependent(this);
		}
	}
	slipType = type;
}

int EWBIKNode3D::getSlipType() {
	return slipType;
}

void EWBIKNode3D::rotateAboutX(float angle, bool orthonormalized) {
	updateGlobal();
	Quat xRot = Quat(getGlobalMBasis().get_x_heading(), angle);
	rotateBy(xRot);
	mark_dirty();
}

void EWBIKNode3D::rotateAboutY(float angle, bool orthonormalized) {
	updateGlobal();
	Quat yRot = Quat(getGlobalMBasis().get_y_heading(), angle);
	rotateBy(yRot);
	mark_dirty();
}

void EWBIKNode3D::rotateAboutZ(float angle, bool orthonormalized) {
	updateGlobal();
	Quat zRot = Quat(getGlobalMBasis().get_z_heading(), angle);
	rotateBy(zRot);
	mark_dirty();
}

void EWBIKNode3D::rotateBy(Quat apply) {
	updateGlobal();
	Ref<EWBIKNode3D> parent_axes = get_parent_axes();
	if (parent_axes.is_valid()) {
		Quat newRot = parent_axes->getGlobalMBasis().get_local_of_rotation(apply);
		getLocalMBasis().rotate_by(newRot);
	} else {
		getLocalMBasis().rotate_by(apply);
	}

	mark_dirty();
}

void EWBIKNode3D::rotateByLocal(Quat apply) {
	updateGlobal();
	if (parent.is_valid()) {
		getLocalMBasis().rotate_by(apply);
	}
	mark_dirty();
}

void EWBIKNode3D::alignLocalsTo(Ref<EWBIKNode3D> targetAxes) {
	getLocalMBasis().adopt_values(targetAxes->localMBasis);
	mark_dirty();
}

void EWBIKNode3D::alignToParent() {
	getLocalMBasis().set_identity();
	mark_dirty();
}

void EWBIKNode3D::rotateToParent() {
	getLocalMBasis().rotate_to(Quat());
	mark_dirty();
}

void EWBIKNode3D::alignGlobalsTo(Ref<EWBIKNode3D> targetAxes) {
	ERR_FAIL_NULL(targetAxes);
	targetAxes->updateGlobal();
	updateGlobal();
	Ref<EWBIKNode3D> parent_axes = get_parent_axes();
	if (parent_axes.is_valid()) {
		parent_axes->getGlobalMBasis().set_to_local_of(targetAxes->globalMBasis, localMBasis);
	} else {
		getLocalMBasis().adopt_values(targetAxes->globalMBasis);
	}
	mark_dirty();
	updateGlobal();
}

void EWBIKNode3D::alignOrientationTo(Ref<EWBIKNode3D> targetAxes) {
	ERR_FAIL_NULL(targetAxes);
	targetAxes->updateGlobal();
	updateGlobal();
	Ref<EWBIKNode3D> parent_axes = get_parent_axes();
	if (parent_axes.is_valid()) {
		getGlobalMBasis().rotate_to(targetAxes->getGlobalMBasis().rotation);
		parent_axes->getGlobalMBasis().set_to_local_of(globalMBasis, localMBasis);
	} else {
		getLocalMBasis().rotate_to(targetAxes->getGlobalMBasis().rotation);
	}
	mark_dirty();
}

void EWBIKNode3D::setGlobalOrientationTo(Quat rotation) {
	updateGlobal();
	if (get_parent_axes() != nullptr) {
		getGlobalMBasis().rotate_to(rotation);
		Ref<EWBIKNode3D> axes = get_parent_axes();
		if (axes.is_valid()) {
			axes->getGlobalMBasis().set_to_local_of(globalMBasis, localMBasis);
		}
	} else {
		getLocalMBasis().rotate_to(rotation);
	}
	mark_dirty();
}

void EWBIKNode3D::registerDependent(Ref<EWBIKAxisDependency> newDependent) {
	//Make sure we don't hit a dependency loop
	Ref<EWBIKNode3D> dependent = newDependent;
	if (dependent.is_valid() && dependent->isAncestorOf(this)) {
		transferToParent(dependent->get_parent_axes());
	}
	if (!dependentsRegistry.find(newDependent)) {
		dependentsRegistry.push_back(newDependent);
	}
}

bool EWBIKNode3D::isAncestorOf(Ref<EWBIKNode3D> potentialDescendent) {
	bool result = false;
	Ref<EWBIKNode3D> cursor = potentialDescendent->get_parent_axes();
	while (cursor != nullptr) {
		if (cursor == this) {
			result = true;
			break;
		} else {
			cursor = cursor->get_parent_axes();
		}
	}
	return result;
}

void EWBIKNode3D::transferToParent(Ref<EWBIKNode3D> newParent) {
	emancipate();
	setParent(newParent);
}

void EWBIKNode3D::emancipate() {
	if (get_parent_axes() != nullptr) {
		updateGlobal();
		Ref<EWBIKNode3D> oldParent = get_parent_axes();
		// for (int32_t i = 0; i < dependentsRegistry.size(); i++) {
		// 	dependentsRegistry[i]->get().parentChangeWarning(this, getParentAxes(), nullptr, nullptr);
		// }
		getLocalMBasis().adopt_values(globalMBasis);
		Ref<EWBIKNode3D> axes = get_parent_axes();
		if (axes.is_valid()) {
			axes->disown(this);
		}
		parent.unref();
		areGlobal = true;
		mark_dirty();
		updateGlobal();
		// for (List<Ref<AxisDependency>>::Element *ad = dependentsRegistry.front(); ad; ad = ad->next()) {
		// 	ad->get()->parentChangeCompletionNotice(this, oldParent, nullptr, nullptr);
		// }
	}
}

void EWBIKNode3D::disown(Ref<EWBIKAxisDependency> child) {
	dependentsRegistry.erase(child);
}

DMIKTransform EWBIKNode3D::getGlobalMBasis() {
	updateGlobal();
	return globalMBasis;
}

DMIKTransform EWBIKNode3D::getLocalMBasis() {
	return localMBasis;
}

void EWBIKNode3D::axis_slip_warning(Ref<EWBIKAxisDependency> globalPriorToSlipping, Ref<EWBIKAxisDependency> globalAfterSlipping, Ref<EWBIKAxisDependency> actualAxis) {
}

void EWBIKNode3D::axis_slip_warning(Ref<EWBIKAxisDependency> p_global_prior_to_slipping, Ref<EWBIKAxisDependency> globalAfterSlipping, Ref<EWBIKAxisDependency> actualAxis, List<Object> dontWarn) {
	updateGlobal();
	if (slipType == NORMAL) {
		if (get_parent_axes() != nullptr) {
			Ref<EWBIKNode3D> globalVals = relativeTo(p_global_prior_to_slipping);
			Ref<EWBIKNode3D> global_prior_to_slipping = p_global_prior_to_slipping;
			globalVals = global_prior_to_slipping->getLocalOf(globalVals);
			getLocalMBasis().adopt_values(globalMBasis);
			mark_dirty();
		}
	} else if (slipType == FORWARD) {
		Ref<EWBIKNode3D> globalAfterVals = relativeTo(globalAfterSlipping);
		// notifyDependentsOfSlip(globalAfterVals, dontWarn);
	}
}

void EWBIKNode3D::axis_slip_completion_notice(Ref<EWBIKAxisDependency> p_global_prior_to_slipping, Ref<EWBIKAxisDependency> p_global_after_slipping, Ref<EWBIKAxisDependency> p_this_axis) {
}

void EWBIKNode3D::slipTo(Ref<EWBIKNode3D> newAxisGlobal, List<Object> dontWarn) {
	updateGlobal();
	Ref<EWBIKNode3D> originalGlobal = getGlobalCopy();
	// notifyDependentsOfSlip(newAxisGlobal, dontWarn);
	Ref<EWBIKNode3D> newVals = newAxisGlobal->getGlobalCopy();

	if (get_parent_axes().is_valid()) {
		Ref<EWBIKNode3D> axes = get_parent_axes();
		if (axes.is_valid()) {
			newVals = axes->getLocalOf(newAxisGlobal);
		}
	}
	alignGlobalsTo(newAxisGlobal);
	mark_dirty();
	updateGlobal();

	// notifyDependentsOfSlipCompletion(originalGlobal, dontWarn);
}

void EWBIKNode3D::slipTo(Ref<EWBIKNode3D> newAxisGlobal) {
	updateGlobal();
	Ref<EWBIKNode3D> originalGlobal = getGlobalCopy();
	// notifyDependentsOfSlip(newAxisGlobal);
	Ref<EWBIKNode3D> newVals = newAxisGlobal->getGlobalCopy();

	if (get_parent_axes() != nullptr) {
		Ref<EWBIKNode3D> axes = get_parent_axes();
		if (axes.is_valid()) {
			newVals = axes->getLocalOf(newVals);
		}
	}
	getLocalMBasis().adopt_values(newVals->globalMBasis);
	dirty = true;
	updateGlobal();

	// notifyDependentsOfSlipCompletion(originalGlobal);
}

// void DMIKNode3D::notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal) {
// 	for (int i = 0; i < dependentsRegistry.size(); i++) {
// 		dependentsRegistry.get(i).get().axisSlipWarning(getGlobalCopy(), newAxisGlobal, this);
// 	}
// }

// void DMIKNode3D::notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn) {
// 	for (int i = 0; i < dependentsRegistry.size(); i++) {
// 		if (!dontWarn.contains(dependentsRegistry.get(i))) {
// 			Ref<AxisDependency> dependant = dependentsRegistry.get(i).get();

// 			//First we check if the dependent extends AbstractAxes
// 			//so we know whether or not to pass the dontWarn list
// 			if (getClass().isAssignableFrom(dependant.getClass())) {
// 				((DMIKNode3D)dependant).axisSlipWarning(getGlobalCopy(), newAxisGlobal, this, dontWarn);
// 			} else {
// 				dependant.axisSlipWarning(getGlobalCopy(), newAxisGlobal, this);
// 			}
// 		}
// 	}
// }
