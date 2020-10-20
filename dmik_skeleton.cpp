#include "dmik_skeleton.h"
#include "core/engine.h"

void DMIKNode3D::updateGlobal() {
	if (dirty) {
		if (areGlobal) {
			globalMBasis.adoptValues(localMBasis);
		} else {
			Ref<DMIKNode3D> parent_axes = getParentAxes();
			if (parent_axes.is_null()) {
				return;
			}
			parent_axes->updateGlobal();
			parent_axes->getGlobalMBasis().setToGlobalOf(localMBasis, globalMBasis);
		}
	}
	dirty = false;
}
void DMIKNode3D::setToLocalOf(DMIKTransform input, DMIKTransform &output) {
	updateGlobal();
	getGlobalMBasis().setToLocalOf(input, output);
}
void DMIKNode3D::setToLocalOf(Vector3 in, Vector3 &out) {
	updateGlobal();
	getGlobalMBasis().setToLocalOf(in, out);
}

void DMIKNode3D::setToLocalOf(Ray in, Ray &out) {
	setToLocalOf(in.position, out.position);
	setToLocalOf(in.normal, out.normal);
}

Vector3 DMIKNode3D::setToLocalOf(Vector3 in) {
	updateGlobal();
	Vector3 result;
	getGlobalMBasis().setToLocalOf(in, result);
	return result;
}

void DMIKNode3D::setToGlobalOf(Ray input, Ray &output) {
	updateGlobal();
	setToGlobalOf(input.position, output.position);
	setToGlobalOf(input.normal, output.normal);
}

void DMIKNode3D::setToGlobalOf(Vector3 input, Vector3 &output) {
	updateGlobal();
	getGlobalMBasis().setToGlobalOf(input, output);
}

Vector3 DMIKNode3D::setToGlobalOf(Vector3 in) {
	updateGlobal();
	getGlobalMBasis().setToGlobalOf(in, in);
	return in;
}

void DMIKNode3D::createTempVars(Vector3 type) {
	workingVector = type;
	tempOrigin = type;
}

Ref<AxisDependency> DMIKNode3D::getParentAxes() {
	if (parent.is_null()) {
		return nullptr;
	} else {
		return parent;
	}
}

void DMIKNode3D::setParent(Ref<DMIKNode3D> par) {
	setParent(par, nullptr);
}

void DMIKNode3D::setParent(Ref<DMIKNode3D> intendedParent, Ref<DMIKNode3D> requestedBy) {
	updateGlobal();
	Ref<DMIKNode3D> oldParent = getParentAxes();
	// forEachDependent(
	// 		(ad)->ad.get().parentChangeWarning(this, oldParent, intendedParent, requestedBy));

	if (intendedParent != nullptr && intendedParent != this) {
		intendedParent->updateGlobal();
		intendedParent->getGlobalMBasis().setToLocalOf(globalMBasis, localMBasis);

		if (oldParent != nullptr) {
			oldParent->disown(this);
		}
		parent = intendedParent;

		Ref<DMIKNode3D> axes = getParentAxes();
		if (axes.is_valid()) {
			axes->registerDependent(this);
		}
		areGlobal = false;
	} else {
		if (oldParent != nullptr)
			oldParent->disown(this);
		parent = nullptr;
		areGlobal = true;
	}
	markDirty();
	updateGlobal();
	// forEachDependent(
	// 		(ad)->ad.get().parentChangeCompletionNotice(this, oldParent, intendedParent, requestedBy));
}

DMIKNode3D::DMIKNode3D(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, Ref<DMIKNode3D> parent, bool customBases) {
	if (!customBases) {
		globalMBasis = parent.is_valid() ? parent->getGlobalMBasis() : DMIKTransform(origin);
		localMBasis = parent.is_valid() ? parent->getLocalMBasis() : DMIKTransform(origin);
		globalMBasis.setIdentity();
		localMBasis.setIdentity();
	}
	if (parent.is_null()) {
		areGlobal = true;
	}
}
DMIKNode3D::DMIKNode3D() {
	areGlobal = true;
	updateGlobal();
}

DMIKNode3D::DMIKNode3D(DMIKTransform p_globalBasis, Ref<DMIKNode3D> p_parent) {
	globalMBasis = p_globalBasis;
	createTempVars(p_globalBasis.getOrigin());
	if (getParentAxes().is_valid()) {
		setParent(p_parent);
	} else {
		areGlobal = true;
		localMBasis = p_globalBasis;
	}
	updateGlobal();
}

Vector3 DMIKNode3D::origin_() {
	updateGlobal();
	tempOrigin = getGlobalMBasis().getOrigin();
	return tempOrigin;
}

Ref<DMIKNode3D> DMIKNode3D::getGlobalCopy() {
    Ref<DMIKNode3D> node;
    node.instance();
    DMIKTransform global_basis = this->globalMBasis;
	node->globalMBasis = global_basis;
	createTempVars(global_basis.getOrigin());
    Ref<DMIKNode3D> parent_axes = this->getParentAxes();
	if (parent_axes.is_valid()) {
		setParent(parent_axes);
	} else {
		areGlobal = true;
		localMBasis = this->globalMBasis;
	}
	updateGlobal();
	return node;
}

int DMIKNode3D::getGlobalChirality() {
	updateGlobal();
	return getGlobalMBasis().chirality;
}

int DMIKNode3D::getLocalChirality() {
	updateGlobal();
	return getLocalMBasis().chirality;
}

bool DMIKNode3D::isGlobalAxisFlipped(int axis) {
	updateGlobal();
	return globalMBasis.isAxisFlipped(axis);
}

bool DMIKNode3D::isLocalAxisFlipped(int axis) {
	return localMBasis.isAxisFlipped(axis);
}

void DMIKNode3D::setRelativeToParent(Ref<DMIKNode3D> par) {
	Ref<DMIKNode3D> axes = getParentAxes();
	if (axes.is_valid()) {
		axes->disown(this);
	}
	parent = par;
	axes = getParentAxes();
	if (axes.is_valid()) {
		axes->registerDependent(this);
	}
	markDirty();
}

bool DMIKNode3D::needsUpdate() {
	if (dirty)
		return true;
	else
		return false;
}

Ray DMIKNode3D::getGlobalOf(Ray in) {
	return Ray(getGlobalOf(in.position), getGlobalOf(in.normal));
}

Vector3 DMIKNode3D::getGlobalOf(Vector3 in) {
	Vector3 result = in;
	setToGlobalOf(in, result);
	return result;
}

Ray DMIKNode3D::getLocalOf(Ray in) {
	Ray result = in;
	result.position = getLocalOf(in.position);
	result.normal = getLocalOf(in.normal);
	return result;
}

Vector3 DMIKNode3D::getLocalOf(Vector3 in) {
	updateGlobal();
	return getGlobalMBasis().getLocalOf(in);
}

void DMIKNode3D::translateByLocal(Vector3 translate) {
	updateGlobal();
	getLocalMBasis().translateBy(translate);
	markDirty();
}

void DMIKNode3D::translateByGlobal(Vector3 translate) {
	if (getParentAxes().is_valid()) {
		updateGlobal();
		translateTo(translate + origin_());
	} else {
		getLocalMBasis().translateBy(translate);
	}

	markDirty();
}

void DMIKNode3D::translateTo(Vector3 translate) {
    Ref<DMIKNode3D> parent_axes = getParentAxes();
	if (parent_axes.is_valid()) {
		updateGlobal();
		getLocalMBasis().translateTo(parent_axes->getGlobalMBasis().getLocalOf(translate));
		markDirty();
	} else {
		updateGlobal();
		getLocalMBasis().translateTo(translate);
		markDirty();
	}
}

void DMIKNode3D::translateTo(Vector3 translate, bool slip) {
	updateGlobal();
	if (slip) {
		Ref<DMIKNode3D> tempAbstractAxes = getGlobalCopy();
		tempAbstractAxes->translateTo(translate);
		slipTo(tempAbstractAxes);
	} else {
		translateTo(translate);
	}
}

void DMIKNode3D::setSlipType(int type) {
    Ref<DMIKNode3D> parent_axes = getParentAxes();
	if (parent_axes.is_valid()) {
		if (type == IGNORE) {
			parent_axes->dependentsRegistry.erase(this);
		} else if (type == NORMAL || type == FORWARD) {
			parent_axes->registerDependent(this);
		}
	}
	slipType = type;
}

int DMIKNode3D::getSlipType() {
	return slipType;
}

void DMIKNode3D::rotateAboutX(float angle, bool orthonormalized) {
	updateGlobal();
	Quat xRot = Quat(getGlobalMBasis().getXHeading(), angle);
	rotateBy(xRot);
	markDirty();
}

void DMIKNode3D::rotateAboutY(float angle, bool orthonormalized) {
	updateGlobal();
	Quat yRot = Quat(getGlobalMBasis().getYHeading(), angle);
	rotateBy(yRot);
	markDirty();
}

void DMIKNode3D::rotateAboutZ(float angle, bool orthonormalized) {
	updateGlobal();
	Quat zRot = Quat(getGlobalMBasis().getZHeading(), angle);
	rotateBy(zRot);
	markDirty();
}

void DMIKNode3D::rotateBy(Quat apply) {
	updateGlobal();
    Ref<DMIKNode3D> parent_axes = getParentAxes();
	if (parent_axes.is_valid()) {
		Quat newRot = parent_axes->getGlobalMBasis().getLocalOfRotation(apply);
		getLocalMBasis().rotateBy(newRot);
	} else {
		getLocalMBasis().rotateBy(apply);
	}

	markDirty();
}

void DMIKNode3D::rotateByLocal(Quat apply) {
	updateGlobal();
	if (parent.is_valid()) {
		getLocalMBasis().rotateBy(apply);
	}
	markDirty();
}

void DMIKNode3D::alignLocalsTo(Ref<DMIKNode3D> targetAxes) {
	getLocalMBasis().adoptValues(targetAxes->localMBasis);
	markDirty();
}

void DMIKNode3D::alignToParent() {
	getLocalMBasis().setIdentity();
	markDirty();
}

void DMIKNode3D::rotateToParent() {
	getLocalMBasis().rotateTo(Quat());
	markDirty();
}

void DMIKNode3D::alignGlobalsTo(Ref<DMIKNode3D> targetAxes) {
    ERR_FAIL_NULL(targetAxes);
	targetAxes->updateGlobal();
	updateGlobal();
    Ref<DMIKNode3D> parent_axes = getParentAxes();
	if (parent_axes.is_valid()) {
		parent_axes->getGlobalMBasis().setToLocalOf(targetAxes->globalMBasis, localMBasis);
	} else {
		getLocalMBasis().adoptValues(targetAxes->globalMBasis);
	}
	markDirty();
	updateGlobal();
}

void DMIKNode3D::alignOrientationTo(Ref<DMIKNode3D> targetAxes) {
    ERR_FAIL_NULL(targetAxes);
	targetAxes->updateGlobal();
	updateGlobal();
    Ref<DMIKNode3D> parent_axes = getParentAxes();
	if (parent_axes.is_valid()) {
		getGlobalMBasis().rotateTo(targetAxes->getGlobalMBasis().rotation);
		parent_axes->getGlobalMBasis().setToLocalOf(globalMBasis, localMBasis);
	} else {
		getLocalMBasis().rotateTo(targetAxes->getGlobalMBasis().rotation);
	}
	markDirty();
}

void DMIKNode3D::setGlobalOrientationTo(Quat rotation) {
	updateGlobal();
	if (getParentAxes() != nullptr) {
		getGlobalMBasis().rotateTo(rotation);
		Ref<DMIKNode3D> axes = getParentAxes();
		if (axes.is_valid()) {
			axes->getGlobalMBasis().setToLocalOf(globalMBasis, localMBasis);
		}
	} else {
		getLocalMBasis().rotateTo(rotation);
	}
	markDirty();
}

void DMIKNode3D::registerDependent(Ref<AxisDependency> newDependent) {
	//Make sure we don't hit a dependency loop
	Ref<DMIKNode3D> dependent = newDependent;
	if (dependent.is_valid() && dependent->isAncestorOf(this)) {
		transferToParent(dependent->getParentAxes());
	}
	if (!dependentsRegistry.find(newDependent)) {
		dependentsRegistry.push_back(newDependent);
	}
}

bool DMIKNode3D::isAncestorOf(Ref<DMIKNode3D> potentialDescendent) {
	bool result = false;
	Ref<DMIKNode3D> cursor = potentialDescendent->getParentAxes();
	while (cursor != nullptr) {
		if (cursor == this) {
			result = true;
			break;
		} else {
			cursor = cursor->getParentAxes();
		}
	}
	return result;
}

void DMIKNode3D::transferToParent(Ref<DMIKNode3D> newParent) {
	emancipate();
	setParent(newParent);
}

void DMIKNode3D::emancipate() {
	if (getParentAxes() != nullptr) {
		updateGlobal();
		Ref<DMIKNode3D> oldParent = getParentAxes();
		// for (int32_t i = 0; i < dependentsRegistry.size(); i++) {
		// 	dependentsRegistry[i]->get().parentChangeWarning(this, getParentAxes(), nullptr, nullptr);
		// }
		getLocalMBasis().adoptValues(globalMBasis);
		Ref<DMIKNode3D> axes = getParentAxes();
		if (axes.is_valid()) {
			axes->disown(this);
		}
		parent = nullptr;
		areGlobal = true;
		markDirty();
		updateGlobal();
		// for (List<Ref<AxisDependency>>::Element *ad = dependentsRegistry.front(); ad; ad = ad->next()) {
		// 	ad->get()->parentChangeCompletionNotice(this, oldParent, nullptr, nullptr);
		// }
	}
}

void DMIKNode3D::disown(Ref<AxisDependency> child) {
	dependentsRegistry.erase(child);
}

DMIKTransform DMIKNode3D::getGlobalMBasis() {
	updateGlobal();
	return globalMBasis;
}

DMIKTransform DMIKNode3D::getLocalMBasis() {
	return localMBasis;
}

void DMIKNode3D::axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis) {
}

void DMIKNode3D::axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis, List<Object> dontWarn) {
	updateGlobal();
	if (slipType == NORMAL) {
		if (getParentAxes() != nullptr) {
			Ref<DMIKNode3D> globalVals = relativeTo(globalPriorToSlipping);
			globalVals = globalPriorToSlipping->getLocalOf(globalVals);
			getLocalMBasis().adoptValues(globalMBasis);
			markDirty();
		}
	} else if (slipType == FORWARD) {
		Ref<DMIKNode3D> globalAfterVals = relativeTo(globalAfterSlipping);
		// notifyDependentsOfSlip(globalAfterVals, dontWarn);
	}
}

void DMIKNode3D::axisSlipCompletionNotice(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> thisAxis) {
}

void DMIKNode3D::slipTo(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn) {
	updateGlobal();
	Ref<DMIKNode3D> originalGlobal = getGlobalCopy();
	// notifyDependentsOfSlip(newAxisGlobal, dontWarn);
	Ref<DMIKNode3D> newVals = newAxisGlobal->getGlobalCopy();

	if (getParentAxes().is_valid()) {
		Ref<DMIKNode3D> axes = getParentAxes();
		if (axes.is_valid()) {
			newVals = axes->getLocalOf(newAxisGlobal);
		}
	}
	alignGlobalsTo(newAxisGlobal);
	markDirty();
	updateGlobal();

	// notifyDependentsOfSlipCompletion(originalGlobal, dontWarn);
}

void DMIKNode3D::slipTo(Ref<DMIKNode3D> newAxisGlobal) {
	updateGlobal();
	Ref<DMIKNode3D> originalGlobal = getGlobalCopy();
	// notifyDependentsOfSlip(newAxisGlobal);
	Ref<DMIKNode3D> newVals = newAxisGlobal->getGlobalCopy();

	if (getParentAxes() != nullptr) {
		Ref<DMIKNode3D> axes = getParentAxes();
		if (axes.is_valid()) {
			newVals = axes->getLocalOf(newVals);
		}
	}
	getLocalMBasis().adoptValues(newVals->globalMBasis);
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
