#include "shadow_skeleton.h"
#include "core/reference.h"

void AbstractAxes::translateTo(Vector3 p_translate) {
	if (this.getParentAxes() != nullptr) {
		updateGlobal();
		getLocalMBasis().translate(getParentAxes().getGlobalMBasis().origin - p_translate);
		markDirty();
	} else {
		this.updateGlobal();
		getLocalMBasis().translate(p_translate);
		markDirty();
	}
}

void AbstractAxes::translateTo(Vector3 p_translate, bool p_slip) {
	this.updateGlobal();
	if (p_slip) {
		AbstractAxes tempAbstractAxes = getGlobalCopy();
		tempAbstractAxes.translateTo(p_translate);
		slipTo(tempAbstractAxes);
	} else {
		translateTo(p_translate);
	}
}

AbstractAxes AbstractAxes::freeCopy() {
	AbstractAxes freeCopy = getLocalMBasis();
	freeCopy.getLocalMBasis().adoptValues(this.localMBasis);
	freeCopy.markDirty();
	freeCopy.updateGlobal();
	return freeCopy;
}

AbstractAxes::CartesianAxes AbstractAxes::attachedCopy(bool p_slipAware) {
	this.updateGlobal();
	CartesianAxes copy = new CartesianAxes(getGlobalMBasis(),
			this.getParentAxes());
	if (!p_slipAware) {
		copy.setSlipType(IGNORE);
	}
	copy.getLocalMBasis().adoptValues(this.localMBasis);
	copy.markDirty();
	return copy;
}

void AbstractAxes::setSlipType(int type) {
	if (this.getParentAxes() != nullptr) {
		if (type == IGNORE) {
			this.getParentAxes().dependentsRegistry.remove(this);
		} else if (type == NORMAL || type == FORWARD) {
			this.getParentAxes().registerDependent(this);
		}
	}
	this.slipType = type;
}

AbstractAxes::CartesianAxes(Vector3 p_origin, Vector3 p_inX, Vector3 p_inY, Vector3 p_inZ, AbstractAxes p_parent) {
	createTempVars(p_origin);

	areGlobal = true;

	localMBasis = new CartesianBasis(p_origin, p_inX, p_inY, p_inZ);
	globalMBasis = new CartesianBasis(p_origin, p_inX, p_inY, p_inZ);

	Vector3 o = p_origin;
	o.set(0, 0, 0);
	Vector3 i = o;
	i.set(1, 1, 1);

	if (parent != nullptr) {
		setParent(parent);
	} else {
		areGlobal = true;
	}
	markDirty();
	updateGlobal();
}

AbstractAxes::CartesianAxes(Transform p_globalBasis, AbstractAxes p_parent) {
	super(globalBasis, parent);
}

Ray AbstractAxes::x_()
{
	this.updateGlobal();
	return this.getGlobalMBasis().getXRay();
}

Ray AbstractAxes::y_() {
	this.updateGlobal();
	return this.getGlobalMBasis().getYRay();
}

Ray AbstractAxes::z_() {
	this.updateGlobal();
	return this.getGlobalMBasis().getZRay();
}

bool AbstractAxes::equals(AbstractAxes ax) {
	this.updateGlobal();
	ax.updateGlobal();

	bool composedRotationsAreEquivalent = getGlobalMBasis().rotation.equals(ax.globalMBasis.rotation);
	bool originsAreEquivalent = getGlobalMBasis().getOrigin().equals(ax.origin_());

	return composedRotationsAreEquivalent && originsAreEquivalent;
}

int AbstractAxes::getSlipType() {
	return this.slipType;
}

void AbstractAxes::rotateAboutX(float p_angle, bool p_orthonormalized) {
	updateGlobal();
	Quat xRot = Quat(getGlobalMBasis().getXHeading(), p_angle);
	rotateBy(xRot);
	markDirty();
}

void AbstractAxes::rotateAboutY(float angle, bool orthonormalized) {
	this.updateGlobal();
	Quat yRot = Quat(getGlobalMBasis().getYHeading(), angle);
	this.rotateBy(yRot);
	this.markDirty();
}

void AbstractAxes::rotateAboutZ(float angle, bool orthonormalized) {
	this.updateGlobal();
	Quat zRot = Quat(getGlobalMBasis().getZHeading(), angle);
	this.rotateBy(zRot);
	this.markDirty();
}

void AbstractAxes::rotateBy(Quat apply) {
	this.updateGlobal();
	if (this.getParentAxes() != nullptr) {
		Quat newRot = this.getParentAxes().getGlobalMBasis().getLocalOfRotation(apply);
		this.getLocalMBasis().rotateBy(newRot);
	} else {
		this.getLocalMBasis().rotateBy(apply);
	}

	this.markDirty();
}

void AbstractAxes::rotateBy(Basis apply) {
	this.updateGlobal();
	if (parent != nullptr) {
		Quat newRot = this.getParentAxes().getGlobalMBasis().getLocalOfRotation(Quat(apply));
		this.getLocalMBasis().rotateBy(newRot);
	} else {
		this.getLocalMBasis().rotateBy(Quat(apply));
	}
	this.markDirty();
}

void AbstractAxes::rotateByLocal(Quat apply) {
	this.updateGlobal();
	if (parent != nullptr) {
		this.getLocalMBasis().rotateBy(apply);
	}
	this.markDirty();
}

void AbstractAxes::alignLocalsTo(AbstractAxes targetAxes) {
	this.getLocalMBasis().adoptValues(targetAxes.localMBasis);
	this.markDirty();
}

void AbstractAxes::alignToParent() {
	this.getLocalMBasis().setIdentity();
	this.markDirty();
}

void AbstractAxes::rotateToParent() {
	this.getLocalMBasis().rotateTo(Quat());
	this.markDirty();
}

void AbstractAxes::alignGlobalsTo(AbstractAxes targetAxes) {
	targetAxes.updateGlobal();
	this.updateGlobal();
	if (this.getParentAxes() != nullptr) {
		getParentAxes().getGlobalMBasis().setToLocalOf(targetAxes.globalMBasis, localMBasis);
	} else {
		this.getLocalMBasis().adoptValues(targetAxes.globalMBasis);
	}
	this.markDirty();
	this.updateGlobal();
}

void AbstractAxes::alignOrientationTo(AbstractAxes targetAxes) {
	targetAxes.updateGlobal();
	this.updateGlobal();
	if (this.getParentAxes() != nullptr) {
		this.getGlobalMBasis().rotateTo(targetAxes.getGlobalMBasis().rotation);
		getParentAxes().getGlobalMBasis().setToLocalOf(this.globalMBasis, this.localMBasis);
	} else {
		this.getLocalMBasis().rotateTo(targetAxes.getGlobalMBasis().rotation);
	}
	this.markDirty();
}

void AbstractAxes::setGlobalOrientationTo(Quat rotation) {
	this.updateGlobal();
	if (this.getParentAxes() != nullptr) {
		this.getGlobalMBasis().rotateTo(rotation);
		getParentAxes().getGlobalMBasis().setToLocalOf(this.globalMBasis, this.localMBasis);
	} else {
		this.getLocalMBasis().rotateTo(rotation);
	}
	this.markDirty();
}

void AbstractAxes::registerDependent(AxisDependency newDependent) {
	//Make sure we don't hit a dependency loop
	if (AbstractAxes.class.isAssignableFrom(newDependent.getClass())) {
		if (((AbstractAxes)newDependent).isAncestorOf(this)) {
			this.transferToParent(((AxisDependency)newDependent).getParentAxes());
		}
	}
	if (dependentsRegistry.indexOf(newDependent) == -1) {
		dependentsRegistry.add(new DependencyReference<AxisDependency>(newDependent));
	}
}

bool AbstractAxes::isAncestorOf(AbstractAxes potentialDescendent) {
	bool result = false;
	AbstractAxes cursor = potentialDescendent.getParentAxes();
	while (cursor != nullptr) {
		if (cursor == this) {
			result = true;
			break;
		} else {
			cursor = cursor.getParentAxes();
		}
	}
	return result;
}

void AbstractAxes::transferToParent(AbstractAxes newParent) {
	this.emancipate();
	this.setParent(newParent);
}

void AbstractAxes::emancipate() {
	if (this.getParentAxes() != nullptr) {
		this.updateGlobal();
		AbstractAxes oldParent = this.getParentAxes();
		for (int32_t i = 0; i < dependentsRegistry.size(); i++) {
			Ref<AbstractAxes> ad = dependentsRegistry[i];
			ad.get().parentChangeWarning(this, this.getParentAxes(), nullptr, nullptr);
		}
		this.getLocalMBasis().adoptValues(this.globalMBasis);
		this.getParentAxes().disown(this);
		this.parent = new Ref<AbstractAxes>(nullptr);
		this.areGlobal = true;
		this.markDirty();
		this.updateGlobal();
		for (DependencyReference<AxisDependency> ad : this.dependentsRegistry) {
			ad.get().parentChangeCompletionNotice(this, oldParent, nullptr, nullptr);
		}
	}
}

void AbstractAxes::emancipate() {
}

void AbstractAxes::disown(AxisDependency child) {
	dependentsRegistry.remove(child);
}

Transform AbstractAxes::getGlobalMBasis() {
	this.updateGlobal();
	return globalMBasis;
}

Transform AbstractAxes::getLocalMBasis() {
	return localMBasis;
}

void AbstractAxes::axisSlipWarning(AbstractAxes p_globalPriorToSlipping, AbstractAxes p_globalAfterSlipping, AbstractAxes p_actualAxis) {
}

void AbstractAxes::axisSlipWarning(AbstractAxes p_globalPriorToSlipping, AbstractAxes p_globalAfterSlipping, AbstractAxes p_actualAxis, List<Object> dontWarn) {
	this.updateGlobal();
	if (this.slipType == NORMAL) {
		if (this.getParentAxes() != nullptr) {
			AbstractAxes globalVals = this.relativeTo(p_globalPriorToSlipping);
			globalVals = p_globalPriorToSlipping.getLocalOf(globalVals);
			this.getLocalMBasis().adoptValues(globalMBasis);
			this.markDirty();
		}
	} else if (this.slipType == FORWARD) {
		AbstractAxes globalAfterVals = this.relativeTo(p_globalAfterSlipping);
		this.notifyDependentsOfSlip(globalAfterVals, dontWarn);
	}
}

void AbstractAxes::axisSlipWarning(AbstractAxes p_globalPriorToSlipping, AbstractAxes p_globalAfterSlipping, AbstractAxes thisAxis) {
}

void AbstractAxes::notifyDependentsOfSlipCompletion(AbstractAxes globalAxisPriorToSlipping) {
	for (int i = 0; i < dependentsRegistry.size(); i++) { //AxisDependancy dependent : dependentsRegistry) {
		dependentsRegistry.get(i).get().axisSlipCompletionNotice(globalAxisPriorToSlipping, this.getGlobalCopy(), this);
	}
}

void AbstractAxes::notifyDependentsOfSlipCompletion(AbstractAxes globalAxisPriorToSlipping, List<Object> dontWarn) {
	for (int i = 0; i < dependentsRegistry.size(); i++) {
		if (!dontWarn.contains(dependentsRegistry.get(i)))
			dependentsRegistry.get(i).get().axisSlipCompletionNotice(globalAxisPriorToSlipping, this.getGlobalCopy(), this);
		else
			System.out.println("skipping: " + dependentsRegistry.get(i));
	}
}

void AbstractAxes::axisSlipCompletionNotice(AbstractAxes p_globalPriorToSlipping, AbstractAxes p_globalAfterSlipping, AbstractAxes p_thisAxis) {
}

void AbstractAxes::axisSlipCompletionNotice(AbstractAxes p_globalPriorToSlipping, AbstractAxes p_globalAfterSlipping, AbstractAxes thisAxis) {
}

void AbstractAxes::parentChangeWarning(AbstractAxes p_warningBy, AbstractAxes p_oldParent, AbstractAxes p_intendedParent, Object p_requestedBy) {
}

void AbstractAxes::slipTo(AbstractAxes newAxisGlobal, List<Object> dontWarn) {
	this.updateGlobal();
	AbstractAxes originalGlobal = this.getGlobalCopy();
	notifyDependentsOfSlip(newAxisGlobal, dontWarn);
	AbstractAxes newVals = newAxisGlobal.getGlobalCopy();

	if (this.getParentAxes() != nullptr) {
		newVals = getParentAxes().getLocalOf(newAxisGlobal);
	}
	this.alignGlobalsTo(newAxisGlobal);
	this.markDirty();
	this.updateGlobal();

	notifyDependentsOfSlipCompletion(originalGlobal, dontWarn);
}

void AbstractAxes::slipTo(AbstractAxes newAxisGlobal) {
	this.updateGlobal();
	AbstractAxes originalGlobal = this.getGlobalCopy();
	notifyDependentsOfSlip(newAxisGlobal);
	AbstractAxes newVals = newAxisGlobal.freeCopy();

	if (this.getParentAxes() != nullptr) {
		newVals = getParentAxes().getLocalOf(newVals);
	}
	this.getLocalMBasis().adoptValues(newVals.globalMBasis);
	this.dirty = true;
	this.updateGlobal();

	notifyDependentsOfSlipCompletion(originalGlobal);
}

void AbstractAxes::notifyDependentsOfSlip(AbstractAxes newAxisGlobal) {
	for (int i = 0; i < dependentsRegistry.size(); i++) {
		dependentsRegistry.get(i).get().axisSlipWarning(this.getGlobalCopy(), newAxisGlobal, this);
	}
}

void AbstractAxes::notifyDependentsOfSlip(AbstractAxes newAxisGlobal, List<Object> dontWarn) {
	for (int i = 0; i < dependentsRegistry.size(); i++) {
		if (!dontWarn.contains(dependentsRegistry.get(i))) {
			AxisDependency dependant = dependentsRegistry.get(i).get();

			//First we check if the dependent extends AbstractAxes
			//so we know whether or not to pass the dontWarn list
			if (this.getClass().isAssignableFrom(dependant.getClass())) {
				((AbstractAxes)dependant).axisSlipWarning(this.getGlobalCopy(), newAxisGlobal, this, dontWarn);
			} else {
				dependant.axisSlipWarning(this.getGlobalCopy(), newAxisGlobal, this);
			}
		} else {
			System.out.println("skipping: " + dependentsRegistry.get(i));
		}
	}
}

void AbstractAxes::parentChangeCompletionNotice(AbstractAxes p_warningBy, AbstractAxes p_oldParent, AbstractAxes p_intendedParent, Object p_requestedBy) {
}

void AbstractAxes::markDirty() {
	if (!this.dirty) {
		this.dirty = true;
		this.markDependentsDirty();
	}
}

void AbstractAxes::markDirty() {
}

void AbstractAxes::markDependentsDirty() {
	forEachDependent((a)->a.get().markDirty());
}

void AbstractAxes::markDependentsDirty() {
}

String AbstractAxes::toString() {
	String global = "Global: " + getGlobalMBasis().toString();
	String local = "Local: " + getLocalMBasis().toString();
	return global + "\n" + local;
}

void AbstractAxes::createTempVars(Vector3 type) {
	workingVector = type.copy();
	tempOrigin = type.copy();
}

AbstractAxes::AbstractAxes(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, AbstractAxes p_parent, bool p_customBases) {
	if (!customBases) {
		globalMBasis = parent != nullptr ? parent.getGlobalMBasis().copy() : new CartesianBasis(origin);
		localMBasis = parent != nullptr ? parent.getLocalMBasis().copy() : new CartesianBasis(origin);
		globalMBasis.setIdentity();
		localMBasis.setIdentity();
	}
	if (parent == nullptr)
		this.areGlobal = true;
}

AbstractAxes::AbstractAxes(Transform p_globalBasis, AbstractAxes p_parent) {
	this.globalMBasis = globalBasis.copy();
	createTempVars(globalBasis.getOrigin());
	if (this.getParentAxes() != nullptr)
		setParent(parent);
	else {
		this.areGlobal = true;
		this.localMBasis = globalBasis.copy();
	}

	this.updateGlobal();
}

AbstractAxes AbstractAxes::getParentAxes() {
	if (this.parent == nullptr)
		return nullptr;
	else
		return this.parent.get();
}

void AbstractAxes::updateGlobal() {
	if (this.dirty) {
		if (this.areGlobal) {
			globalMBasis.adoptValues(this.localMBasis);
		} else {
			getParentAxes().updateGlobal();
			getParentAxes().getGlobalMBasis().setToGlobalOf(this.localMBasis, this.globalMBasis);
		}
	}
	dirty = false;
}

Vector3 AbstractAxes::origin_() {
	this.updateGlobal();
	tempOrigin.set(this.getGlobalMBasis().getOrigin());
	return tempOrigin;
}

AbstractAxes::CartesianAxes AbstractAxes::getGlobalCopy() {
	return new CartesianAxes(getGlobalMBasis(), this.getParentAxes());
}

void AbstractAxes::setParent(AbstractAxes intendedParent, Object requestedBy) {
	this.updateGlobal();
	AbstractAxes oldParent = this.getParentAxes();
	forEachDependent(
			(ad)->ad.get().parentChangeWarning(this, oldParent, intendedParent, requestedBy));

	if (intendedParent != nullptr && intendedParent != this) {
		intendedParent.updateGlobal();
		intendedParent.getGlobalMBasis().setToLocalOf(globalMBasis, localMBasis);

		if (oldParent != nullptr)
			oldParent.disown(this);
		this.parent = new Ref<AbstractAxes>(intendedParent);

		this.getParentAxes().registerDependent(this);
		this.areGlobal = false;
	} else {
		if (oldParent != nullptr)
			oldParent.disown(this);
		this.parent = new Ref<AbstractAxes>(nullptr);
		this.areGlobal = true;
	}
	this.markDirty();
	this.updateGlobal();

	forEachDependent(
			(ad)->ad.get().parentChangeCompletionNotice(this, oldParent, intendedParent, requestedBy));
}

void AbstractAxes::setParent(AbstractAxes par) {
	setParent(par, nullptr);
}

void AbstractAxes::forEachDependent(Ref<AbstractAxes>> action) {
	Iterator<DependencyReference<AxisDependency>> i = dependentsRegistry.front();
	while (i.hasNext()) {
		DependencyReference<AxisDependency> dr = i.next();
		if (dr.get() != nullptr) {
			action.accept(dr);
		} else {
			i.remove();
		}
	}
}

int AbstractAxes::getGlobalChirality() {
	this.updateGlobal();
	return this.getGlobalMBasis().chirality;
}

int AbstractAxes::getLocalChirality() {
	this.updateGlobal();
	return this.getLocalMBasis().chirality;
}

bool AbstractAxes::isGlobalAxisFlipped(int axis) {
	this.updateGlobal();
	return globalMBasis.isAxisFlipped(axis);
}

bool AbstractAxes::isLocalAxisFlipped(int axis) {
	return localMBasis.isAxisFlipped(axis);
}

void AbstractAxes::setRelativeToParent(AbstractAxes par) {
	if (this.getParentAxes() != nullptr)
		this.getParentAxes().disown(this);
	this.parent = new Ref<AbstractAxes>(par);
	this.areGlobal = false;
	this.getParentAxes().registerDependent(this);
	this.markDirty();
}

bool AbstractAxes::needsUpdate() {
	if (this.dirty)
		return true;
	else
		return false;
}

void AbstractAxes::setToGlobalOf(Ray input, Ray output) {
	this.updateGlobal();
	this.setToGlobalOf(input.p1(), output.p1());
	this.setToGlobalOf(input.p2(), output.p2());
}

void AbstractAxes::setToGlobalOf(Vector3 input, Vector3 output) {
	this.updateGlobal();
	getGlobalMBasis().setToGlobalOf(input, output);
}

Vector3 AbstractAxes::setToGlobalOf(Vector3 in) {
	this.updateGlobal();
	getGlobalMBasis().setToGlobalOf(in, in);
	return in;
}

Ray AbstractAxes::getGlobalOf(Ray in) {
	return new Ray(this.getGlobalOf(in.p1()), this.getGlobalOf(in.p2()));
}

Vector3 AbstractAxes::getGlobalOf(Vector3 in) {
	Vector3 result = in.copy();
	setToGlobalOf(in, result);
	return result;
}

AbstractAxes AbstractAxes::relativeTo(AbstractAxes in) {
	// TODO Auto-generated method stub
	return nullptr;
}

void AbstractAxes::setToLocalOf(Transform input, Transform output) {
	this.updateGlobal();
	this.getGlobalMBasis().setToLocalOf(input, output);
}

void AbstractAxes::setToLocalOf(Ray in, Ray out) {
	this.setToLocalOf(in.p1(), out.p1());
	this.setToLocalOf(in.p2(), out.p2());
}

void AbstractAxes::setToLocalOf(Vector3 in, Vector3 out) {
	this.updateGlobal();
	this.getGlobalMBasis().setToLocalOf(in, out);
}

Vector3 AbstractAxes::setToLocalOf(Vector3 in) {
	this.updateGlobal();
	V result = (V)in.copy();
	this.getGlobalMBasis().setToLocalOf(in, result);
	in.set(result);
	return result;
}

AbstractAxes AbstractAxes::getLocalOf(AbstractAxes input) {
	// TODO Auto-generated method stub
	return nullptr;
}

Vector3 AbstractAxes::getLocalOf(Vector3 in) {
	this.updateGlobal();
	return getGlobalMBasis().getLocalOf(in);
}

void AbstractAxes::translateByGlobal(Vector3 translate) {
	if (this.getParentAxes() != nullptr) {
		this.updateGlobal();
		this.translateTo(translate.addCopy(this.origin_()));
	} else {
		getLocalMBasis().translateBy(translate);
	}

	this.markDirty();
}


void createTempVars(Vector3 type) {
	workingVector = typef

	// @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them, AbstractAxes p_parent) {
	//	this meters will be automatically ADDED to the origin in order to create this basis vector, given as an offset from this base 's origin in global coordinatesparam inZ the direction of the Z basis vector in global coordinates, given as an offset from this base' s origin in global coordinates.*@param forceOrthoNormality *@param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.*/ AbstractAxes(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, AbstractAxes p_parent, bool p_customBases) {
	if (!customBases) {
		globalMBasis = parent != nullptr ? parent.getGlobalMBasis().copy() : new CartesianBasis(origin);
		localMBasis = parent != nullptr ? parent.getLocalMBasis().copy() : new CartesianBasis(origin);
		globalMBasis.setIdentity();
		localMBasis.setIdentity();
	}
	if (parent == nullptr) {
		this.areGlobal = true;
	}
}

AbstractAxes getParentAxes() {
	if (this.parent == nullptr)
		return nullptr;
	else
		return AbstractAxes::getLocalOf()
}
{
	CartesianBasis newBasis = new CartesianBasis((CartesianBasis)input);
	getGlobalMBasis().setToLocalOf(input, newBasis);
	return (Transform)newBasis;
}
