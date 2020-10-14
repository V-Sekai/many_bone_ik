#include "core/engine.h"
#include "core/reference.h"
#include "ray.h"

class AxisDependency : public Reference {
public:
	virtual void emancipate(){};
	virtual void axisSlipWarning(Ref<AxisDependency> globalPriorToSlipping, Ref<AxisDependency> globalAfterSlipping, Ref<AxisDependency> thisAxis){};
	virtual void axisSlipCompletionNotice(Ref<AxisDependency> globalPriorToSlipping, Ref<AxisDependency> globalAfterSlipping, Ref<AxisDependency> thisAxis){};
	virtual void parentChangeWarning(Ref<AxisDependency> warningBy, Ref<AxisDependency> oldParent, Ref<AxisDependency> intendedParent, Object requestedBy){};
	virtual void parentChangeCompletionNotice(Ref<AxisDependency> warningBy, Ref<AxisDependency> oldParent, Ref<AxisDependency> intendedParent, Object requestedBy){};
	virtual void markDirty(){};
	virtual void markDependentsDirty(){};
	virtual Ref<AxisDependency> getParentAxes() = 0;
};

class DMIKTransform {
protected:
	Vector3 xBase = Vector3(1,0,0);
	Vector3 yBase = Vector3(0,1,0);
	Vector3 zBase = Vector3(0,0,1);
	Ray xRay = Ray(Vector3(0,0,0), Vector3(1,0,0));
	Ray yRay = Ray(Vector3(0,0,0), Vector3(0,1,0));
	Ray zRay = Ray(Vector3(0,0,0), Vector3(0,0,1));

public:
	const static int LEFT = -1;
	const static int RIGHT = 1;
	const int chirality = RIGHT;
	const static int NONE = -1;
	const static int X = 0;
	const static int Y = 1;
	const static int Z = 2;
	Quat rotation;
	Quat inverseRotation;
	/**
	 * a vector representing the translation of this basis relative to its parent. 
	 */
	Vector3 translate;
private:
	void updateRays() {
		xRay.position = this->translate;
		xRay.normal = xBase;
		yRay.position = this->translate;
		yRay.normal = yBase;
		zRay.position = this->translate;
		zRay.normal = zBase;

		rotation.set_euler(xRay.normal);
		rotation.set_euler(yRay.normal);
		rotation.set_euler(zRay.normal);

		xRay.normal += this->translate;
		yRay.normal += this->translate;
		zRay.normal += this->translate;
	}
	Quat createPrioritizedRotation(Vector3 xHeading, Vector3 yHeading, Vector3 zHeading) {
		Quat toYZ = Quat(yBase * zBase * yHeading * zHeading);
		Vector3 tempV = toYZ.get_euler() * yBase;
		Quat toY = Quat(tempV * yHeading);
		return toY * toYZ;
	}
	void set(Vector3 x, Vector3 y, Vector3 z) {
		xBase = translate;
		yBase = translate;
		zBase = translate;
		xBase = Vector3(1, 0, 0);
		yBase = Vector3(0, 1, 0);
		zBase = Vector3(0, 0, 1);
		Vector3 zero;
		xRay.position = zero;
		xRay.normal = xBase;
		yRay.position = zero;
		yRay.normal = yBase;
		zRay.position = zero;
		zRay.normal = zBase;
		this->rotation = createPrioritizedRotation(x, y, z);
		this->refreshPrecomputed();
	}
public:
	/**
	 * Initialize this basis at the origin. The basis will be right handed by default. 
	 * @param origin
	 */
	DMIKTransform(Vector3 origin) {
		translate = origin;
		xBase = origin;
		yBase = origin;
		zBase = origin;
		xBase *= Vector3(1, 0, 0);
		yBase *= Vector3(0, 1, 0);
		zBase *= Vector3(0, 0, 1);
		Vector3 zero;
		xRay = Ray(zero, xBase);
		yRay = Ray(zero, yBase);
		zRay = Ray(zero, zBase);
		refreshPrecomputed();
	}

	DMIKTransform(const DMIKTransform &input) {
		translate = input.translate;
		xBase = translate;
		yBase = translate;
		zBase = translate;
		xBase = Vector3(1, 0, 0);
		yBase = Vector3(0, 1, 0);
		zBase = Vector3(0, 0, 1);
		Vector3 zero = translate;
		zero = Vector3(0, 0, 0);
		xRay = Ray(zero, xBase);
		yRay = Ray(zero, yBase);
		zRay = Ray(zero, zBase);
		this->adoptValues(input);
	}

	/**
	 * Initialize this basis at the origin.
	 *  The basis will be backed by a rotation object which presumes right handed chirality. 
	 *  Therefore, the rotation object will align so its local XY plane aligns with this basis' XY plane
	 *  Afterwards, it will check chirality, and if the basis isn't right handed, this class will assume the
	 *  z-axis is the one that's been flipped. 
	 *  
	 *  If you want to manually specify which axis has been flipped 
	 *  (so that the rotation object aligns with respect to the plane formed 
	 *  by the other two basis vectors) then use the constructor dedicated for that purpose
	 * @param origin
	 * @param x basis vector direction
	 * @param y basis vector direction
	 * @param z basis vector direction
	 */
	DMIKTransform(Vector3 origin, Vector3 x, Vector3 y, Vector3 z) {
		this->translate = origin;
		xRay = Ray(origin, origin);
		yRay = Ray(origin, origin);
		zRay = Ray(origin, origin);
		this->set(x, y, z);
	}

	/**
	 * Initialize this basis at the origin defined by the base of the @param x Ray.
	 * 
	 *  The basis will be backed by a rotation object which presumes right handed chirality. 
	 *  Therefore, the rotation object will align so its local XY plane aligns with this basis' XY plane
	 *  Afterwards, it will check chirality, and if the basis isn't right handed, this class will assume the
	 *  z-axis is the one that's been flipped.
	 *   
	 *  If you want to manually specify which axis has been flipped 
	 *  (so that the rotation object aligns with respect to the plane formed 
	 *  by the other two basis vectors) then use the constructor dedicated for that purpose
	 *  
	 *  
	 * @param x basis Ray 
	 * @param y basis Ray 
	 * @param z basis Ray 
	 */
	DMIKTransform(Ray x, Ray y, Ray z) {
		this->translate = x.position;
		xRay = x;
		yRay = y;
		zRay = z;
		Vector3 xDirNew = x.heading();
		Vector3 yDirNew = y.heading();
		Vector3 zDirNew = z.heading();
		xDirNew.normalize();
		yDirNew.normalize();
		zDirNew.normalize();
		set(xDirNew, yDirNew, zDirNew);
	}

	/**
	 * takes on the same values as the input basis. 
	 * @param in
	 */
	void adoptValues(DMIKTransform in) {
		this->translate = in.translate;
		this->rotation = in.rotation;
		xBase = translate * Vector3(1, 0, 0);
		yBase = translate * Vector3(0, 1, 0);
		zBase = translate * Vector3(0, 0, 1);
		xRay = in.xRay;
		yRay = in.yRay;
		zRay = in.zRay;
		this->refreshPrecomputed();
	}
	void setIdentity() {
		this->translate = Vector3();
		xBase = Vector3(1, 0, 0);
		yBase = Vector3(0, 1, 0);
		zBase = Vector3(0, 0, 1);
		this->xRay.position = this->translate;
		this->xRay.normal = xBase;
		this->yRay.position = this->translate;
		this->yRay.normal = yBase;
		this->zRay.position = this->translate;
		this->zRay.normal = zBase;
		this->rotation = Quat();
		refreshPrecomputed();
	}

	Quat getLocalOfRotation(Quat inRot) {
		Quat resultNew = inverseRotation * inRot * rotation;
		return resultNew;
	}

	void setToLocalOf(DMIKTransform global_input, DMIKTransform local_output) {
		local_output.translate = this->getLocalOf(global_input.translate);
		inverseRotation *= global_input.rotation * local_output.rotation;
		local_output.refreshPrecomputed();
	}

	void refreshPrecomputed() {
		this->rotation = inverseRotation.inverse();
		this->updateRays();
	}

	Vector3 getLocalOf(Vector3 v) {
		Vector3 result = v;
		setToLocalOf(v, result);
		return result;
	}

	void setToLocalOf(Vector3 input, Vector3 &output) {
		output = input;
		output -= this->translate;
		output = inverseRotation.get_euler() * output;
	}

	void rotateTo(Quat newRotation) {
		this->rotation = newRotation;
		this->refreshPrecomputed();
	}

	void rotateBy(Quat addRotation) {
		this->rotation *= addRotation;
		this->refreshPrecomputed();
	}

	/**
	 * sets globalOutput such that the result of 
	 * this->getLocalOf(globalOutput) == localInput. 
	 * 
	 * @param localInput
	 * @param globalOutput
	 */
	void setToGlobalOf(DMIKTransform localInput, DMIKTransform globalOutput) {
		this->rotation *= localInput.rotation * globalOutput.rotation;
		this->setToGlobalOf(localInput.translate, globalOutput.translate);
		globalOutput.refreshPrecomputed();
	}

	void setToGlobalOf(Vector3 input, Vector3 &output) {
		rotation = Quat(input);
		output += this->translate;
	}

	void translateBy(Vector3 transBy) {
		this->translate.x += transBy.x;
		this->translate.y += transBy.y;
		this->translate.z += transBy.z;
		updateRays();
	}

	void translateTo(Vector3 newOrigin) {
		this->translate.x = newOrigin.x;
		this->translate.y = newOrigin.y;
		this->translate.z = newOrigin.z;
		updateRays();
	}

	Ray getXRay() {
		return xRay;
	}

	Ray getYRay() {
		return yRay;
	}

	Ray getZRay() {
		return zRay;
	}

	Vector3 getXHeading() {
		return this->xRay.heading();
	}

	Vector3 getYHeading() {
		return this->yRay.heading();
	}

	Vector3 getZHeading() {
		return this->zRay.heading();
	}

	Vector3 getOrigin() {
		return translate;
	}

	/**
	 * true if the input axis should be multiplied by negative one after rotation. 
	 * By default, this always returns false. But can be overridden for more advanced implementations
	 * allowing for reflection transformations. 
	 * @param axis
	 * @return true if axis should be flipped, false otherwise. Default is false. 
	 */
	bool isAxisFlipped(int axis) {
		return false;
	}

	/**
	 * @return a precomputed inverse of the rotation represented by this basis object.
	 */
	Quat getInverseRotation() {
		return this->inverseRotation;
	}
};

// class DMIKNode3D : public : AxisDependency {
// private:
// 	Ref<AxisDependency> parent;
// 	int slipType = 0;

// protected:
// 	Vector3 workingVector;
// 	bool areGlobal = true;

// public:
// 	static const int NORMAL = 0, IGNORE = 1, FORWARD = 2;
// 	static const int RIGHT = 1, LEFT = -1;
// 	static const int X = 0, Y = 1, Z = 2;
// 	bool debug = false;
// 	DMIKTransform localMBasis;
// 	DMIKTransform globalMBasis;
// 	bool dirty = true;

// 	Vector3 tempOrigin;

// 	List<Ref<AxisDependency>> dependentsRegistry;
// 	void createTempVars(Vector3 type);

// 	Ref<AxisDependency> getParentAxes();

// 	void updateGlobal();

// 	/**
//      * Sets the parentAxes for this axis globally.
//      * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz
//      * change.
//      *
//      *   @param intendedParent the new parent Axes
//      *   @param requestedBy the object making thisRequest, will be passed on to parentChangeWarning
//      *   for any AxisDependancy objects registered with this Ref<AbstractAxes>  (can be nullptr if not important)
//      **/
// 	void setParent(Ref<DMIKNode3D> intendedParent, Object requestedBy);

// 	/**
//      * Sets the parentAxes for this axis globally.
//      * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz
//      * change.
//      *
//      *   @param par the new parent Axes
//      **/
// 	void setParent(Ref<DMIKNode3D> par);

// 	Vector3 origin_();

// 	/**
// 		 * Make a GlobalCopy of these Axes.
// 		 * @return
// 		 */
// 	Ref<DMIKNode3D> getGlobalCopy();

// 	int getGlobalChirality();

// 	int getLocalChirality();

// 	/**
// 		 * True if the input axis of this Axes object in global coordinates should be multiplied by negative one after rotation.
// 		 * By default, this always returns false. But can be overridden for more advanced implementations
// 		 * allowing for reflection transformations.
// 		 * @param axis
// 		 * @return true if axis should be flipped, false otherwise. Default is false.
// 		 */
// 	bool isGlobalAxisFlipped(int axis);

// 	/**
// 		 * True if the input axis of this Axes object in local coordinates should be multiplied by negative one after rotation.
// 		 * By default, this always returns false. But can be overridden for more advanced implementations
// 		 * allowing for reflection transformations.
// 		 * @param axis
// 		 * @return true if axis should be flipped, false otherwise. Default is false.
// 		 */
// 	bool isLocalAxisFlipped(int axis);

// 	/**
// 		 * Sets the parentAxes for this axis locally.
// 		 * in other words, lx, ly, lz remain unchanged, but globalX, globalY, and globalZ
// 		 * change.
// 		 *
// 		 * if setting this parent would result in a dependency loop, then the input Axes
// 		 * parent is set to this Axes' parent, prior to this axes setting the input axes
// 		 * as its parent.
// 		 **/
// 	void setRelativeToParent(Ref<DMIKNode3D> par);

// 	bool needsUpdate();

// 	/**
// 		 * Given a vector in this axes local coordinates, returns the vector's position in global coordinates.
// 		 * @param in
// 		 * @return
// 		 */
// 	Vector3 getGlobalOf(Vector3 in);

// 	/**
// 		 *  Given a vector in this axes local coordinates, modifies the vector's values to represent its position global coordinates.
// 		 * @param in
// 		 * @return a reference to this the @param in object.
// 		 */
// 	Vector3 setToGlobalOf(
// 			Vector3 in);

// 	/**
// 		 *  Given an input vector in this axes local coordinates, modifies the output vector's values to represent the input's position in global coordinates.
// 		 * @param in
// 		 */
// 	void setToGlobalOf(Vector3 input, Vector3 &output);

// 	/**
// 		 *  Given an input sgRay in this axes local coordinates, modifies the output Rays's values to represent the input's in global coordinates.
// 		 * @param in
// 		 */
// 	void setToGlobalOf(Ray input, Ray &output);

// 	Ray getGlobalOf(Ray in);

// 	/**
// 		 * returns an axis representing the global location of this axis if the input axis were its parent.
// 		 * @param in
// 		 * @return
// 		 */
// 	Ref<DMIKNode3D> virtual relativeTo(Ref<DMIKNode3D> in) = 0;

// 	Vector3 getLocalOf(Vector3 in);

// 	/**
// 		 *  Given a vector in global coordinates, modifies the vector's values to represent its position in theseAxes local coordinates.
// 		 * @param in
// 		 * @return a reference to the @param in object.
// 		 */
// 	Vector3 setToLocalOf(Vector3 in);

// 	/**
// 		 *  Given a vector in global coordinates, modifies the vector's values to represent its position in theseAxes local coordinates.
// 		 * @param in
// 		 */
// 	void setToLocalOf(Vector3 in, Vector3 out);

// 	/**
// 		 *  Given a sgRay in global coordinates, modifies the sgRay's values to represent its position in theseAxes local coordinates.
// 		 * @param in
// 		 */
// 	void setToLocalOf(Ray in, Ray out);

// 	void setToLocalOf(AbstractBasis input, AbstractBasis &output);

// 	Ray getLocalOf(Ray in);
// 	virtual Ref<DMIKNode3D> getLocalOf(Ref<DMIKNode3D> input) = 0;

// 	virtual DMIKTransform getLocalOf(AbstractBasis input) = 0;

// 	void translateByLocal(Vector3 translate);
// 	void translateByGlobal(Vector3 translate);
// 	void translateTo(Vector3 translate, bool slip);

// 	void translateTo(Vector3 translate);

// 	/**
// 		 * @return a copy of these axes that does not refer to any parent.
// 		 * Because there is no parent, the copy's global coordinates and local coordinates will be equivalent to each other.
// 		 * The copy's local coordinates will also be equivalent to the original's local coordinates. However, the copy's
// 		 * global coordinates will likely be drastically different from the original's global coordinates.
// 		 *
// 		 */
// 	virtual Ref<DMIKNode3D> freeCopy() = 0;

// 	/**
// 		 *  return a ray / segment representing this Axes global x basis position and direction and magnitude
// 		 * @return a ray / segment representing this Axes global x basis position and direction and magnitude
// 		 */
// 	virtual Ray x_() = 0;

// 	/**
// 		 *  return a ray / segment representing this Axes global y basis position and direction and magnitude
// 		 * @return a ray / segment representing this Axes global y basis position and direction and magnitude
// 		 */
// 	virtual Ray y_() = 0;

// 	/**
// 		 *  return a ray / segment representing this Axes global z basis position and direction and magnitude
// 		 * @return a ray / segment representing this Axes global z basis position and direction and magnitude
// 		 */
// 	virtual Ray z_() = 0;

// 	/**
// 	 * Creates an exact copy of this Axes object. Attached to the same parent as this Axes object
// 	 * @param slipAware
// 	 * @return
// 	 */
// 	virtual Ref<DMIKNode3D> attachedCopy(bool slipAware) = 0;
// 	void setSlipType(int type);
// 	int getSlipType();
// 	void rotateAboutX(float angle, bool orthonormalized);
// 	void rotateAboutY(float angle, bool orthonormalized);
// 	void rotateAboutZ(float angle, bool orthonormalized);

// 	/**
// 		 * Rotates the bases around their origin in global coordinates
// 		 * @param rotation
// 		 */
// 	void rotateBy(Quat apply);

// 	/**
// 		 * rotates the bases around their origin in Local coordinates
// 		 * @param rotation
// 		 */
// 	void rotateByLocal(Quat apply);

// 	/**
// 		 * sets these axes to have the same orientation and location relative to their parent
// 		 * axes as the input's axes do to the input's parent axes.
// 		 *
// 		 * If the axes on which this function is called are orthonormal,
// 		 * this function normalizes and orthogonalizes them regardless of whether the targetAxes are orthonormal.
// 		 *
// 		 * @param targetAxes the Axes to make this Axis identical to
// 		 */
// 	void alignLocalsTo(Ref<DMIKNode3D> targetAxes);

// 	/**
// 		 * sets the bases to the Identity basis and Identity rotation relative to its parent, and translates
// 		 * its origin to the parent's origin.
// 		 *
// 		 * be careful calling this method, as it destroys any shear / scale information.
// 		 */
// 	void alignToParent();

// 	/**
// 		 * rotates and translates the axes back to its parent, but maintains
// 		 * its shear, translate and scale attributes.
// 		 */
// 	void rotateToParent();

// 	/**
// 		 * sets these axes to have the same global orientation as the input Axes.
// 		 * these Axes lx, ly, and lz headings will differ from the target ages,
// 		 * but its gx, gy, and gz headings should be identical unless this
// 		 * axis is orthonormalized and the target axes are not.
// 		 * @param targetAxes
// 		 */
// 	void alignGlobalsTo(Ref<DMIKNode3D> targetAxes);
// 	void alignOrientationTo(Ref<DMIKNode3D> targetAxes);

// 	/**
// 		 * updates the axes object such that its global orientation
// 		 * matches the given Quat object.
// 		 * @param rotation
// 		 */
// 	void setGlobalOrientationTo(Quat rotation);
// 	void registerDependent(Ref<AxisDependency> newDependent);
// 	bool isAncestorOf(Ref<DMIKNode3D> potentialDescendent);

// 	/**
// 		 * unregisters this Ref<AbstractAxes> from its current parent and
// 		 * registers it to a new parent without changing its global position or orientation
// 		 * when doing so.
// 		 * @param newParent
// 		 */

// 	void transferToParent(Ref<DMIKNode3D> newParent);

// 	/**
// 		 * unregisters this Ref<AbstractAxes> from its parent,
// 		 * but keeps its global position the same.
// 		 */
// 	void emancipate();
// 	void disown(Ref<AxisDependency> child);
// 	DMIKTransform getGlobalMBasis();
// 	DMIKTransform getLocalMBasis();
// 	void axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis, List<Object> dontWarn);

// 	/**
// 		 * if the input axes have have the same global
// 		 * values as these axes, returns true, otherwise, returns false.
// 		 *
// 		 * This function is orthonormality aware. Meaning, if the orthonormality
// 		 * constraint is enabled on either axes, that axes' orthonormal version
// 		 * will be used in the comparison.
// 		 * @param ax
// 		 */
// 	bool equals(Ref<DMIKNode3D> ax);
// 	void axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis);
// 	void axisSlipCompletionNotice(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> thisAxis);
// 	void slipTo(Ref<DMIKNode3D> newAxisGlobal);
// 	void slipTo(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn);
// 	void notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn);
// 	void notifyDependentsOfSlipCompletion(Ref<DMIKNode3D> globalAxisPriorToSlipping, List<Object> dontWarn) {
// 		for (int i = 0; i < dependentsRegistry.size(); i++) {
// 			if (dontWarn.find(dependentsRegistry[i]) == -1) {
// 				dependentsRegistry[i]->get().axisSlipCompletionNotice(globalAxisPriorToSlipping, getGlobalCopy(), this);
// 			}
// 		}
// 	}
// 	void notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal);
// 	void notifyDependentsOfSlipCompletion(Ref<DMIKNode3D> globalAxisPriorToSlipping) {
// 		for (int i = 0; i < dependentsRegistry.size(); i++) {
// 			dependentsRegistry[i]->axisSlipCompletionNotice(globalAxisPriorToSlipping, getGlobalCopy(), this);
// 		}
// 	}

// 	void markDirty() {
// 		if (!dirty) {
// 			dirty = true;
// 			markDependentsDirty();
// 		}
// 	}

// 	void markDependentsDirty() {
// 		List<Ref<AxisDependency>>::Element *i = dependentsRegistry.front();
// 		while (i) {
// 			Ref<AxisDependency> dr = i->get();
// 			if (dr.is_valid()) {
// 				dr->markDirty()
// 			}
// 			i = i.next();
// 		}
// 	}

// 	/**
//      * @param globalMBasis a Basis object for this Axes to adopt the values of
//      * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
//      */
// 	DMIKNode3D(AbstractBasis p_globalBasis, Ref<DMIKNode3D> p_parent);

// 	/**
// 		 * @param origin the center of this axes basis. The basis vector parameters will be automatically ADDED to the origin in order to create this basis vector.
// 		 * @param inX the direction of the X basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
// 		 * @param inY the direction of the Y basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
// 		 * @param inZ the direction of the Z basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
// 		 * @param forceOrthoNormality
// 		 * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
// 		 */
// 	DMIKNode3D(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, Ref<DMIKNode3D> parent, bool customBases);
// };

// void DMIKNode3D::setToLocalOf(DMIKTransform input, DMIKTransform &output) {
// 	updateGlobal();
// 	getGlobalMBasis().setToLocalOf(input, output);
// }

// void DMIKNode3D::setToLocalOf(Ray in, Ray out) {
// 	setToLocalOf(in.position, out.position);
// 	setToLocalOf(in.normal, out.normal);
// }

// void DMIKNode3D::setToLocalOf(Vector3 in, Vector3 &out) {
// 	updateGlobal();
// 	getGlobalMBasis().setToLocalOf(in, out);
// }

// Vector3 DMIKNode3D::setToLocalOf(Vector3 in) {
// 	updateGlobal();
// 	Vector3 result = in;
// 	getGlobalMBasis().setToLocalOf(in, result);
// 	in.set(result);
// 	return result;
// }

// void DMIKNode3D::setToGlobalOf(Ray input, Ray &output) {
// 	updateGlobal();
// 	setToGlobalOf(input.position, output.position);
// 	setToGlobalOf(input.normal, output.normal);
// }

// void DMIKNode3D::setToGlobalOf(Vector3 input, Vector3 &output) {
// 	updateGlobal();
// 	getGlobalMBasis().setToGlobalOf(input, output);
// }

// Vector3 DMIKNode3D::setToGlobalOf(Vector3 in) {
// 	updateGlobal();
// 	getGlobalMBasis().setToGlobalOf(in, in);
// 	return in;
// }

// void DMIKNode3D::createTempVars(Vector3 type) {
// 	workingVector = type;
// 	tempOrigin = type;
// }

// Ref<AxisDependency> DMIKNode3D::getParentAxes() {
// 	if (parent.is_null()) {
// 		return nullptr;
// 	} else {
// 		return parent;
// 	}
// }

// void DMIKNode3D::updateGlobal() {
// 	if (dirty) {
// 		if (areGlobal) {
// 			globalMBasis.adoptValues(localMBasis);
// 		} else {
// 			getParentAxes()->updateGlobal();
// 			getParentAxes()->getGlobalMBasis().setToGlobalOf(localMBasis, globalMBasis);
// 		}
// 	}
// 	dirty = false;
// }

// void DMIKNode3D::setParent(Ref<DMIKNode3D> par) {
// 	setParent(par, nullptr);
// }

// void DMIKNode3D::setParent(Ref<DMIKNode3D> intendedParent, Object requestedBy) {
// 	updateGlobal();
// 	Ref<DMIKNode3D> oldParent = getParentAxes();
// 	forEachDependent(
// 			(ad)->ad.get().parentChangeWarning(this, oldParent, intendedParent, requestedBy));

// 	if (intendedParent != nullptr && intendedParent != this) {
// 		intendedParent.updateGlobal();
// 		intendedParent.getGlobalMBasis().setToLocalOf(globalMBasis, localMBasis);

// 		if (oldParent != nullptr) {
// 			oldParent.disown(this);
// 		}
// 		parent = intendedParent;

// 		getParentAxes().registerDependent(this);
// 		areGlobal = false;
// 	} else {
// 		if (oldParent != nullptr)
// 			oldParent.disown(this);
// 		parent = nullptr;
// 		areGlobal = true;
// 	}
// 	markDirty();
// 	updateGlobal();
// 	forEachDependent(
// 			(ad)->ad.get().parentChangeCompletionNotice(this, oldParent, intendedParent, requestedBy));
// }

// DMIKNode3D::DMIKNode3D(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, Ref<DMIKNode3D> parent, bool customBases) {
// 	if (!customBases) {
// 		globalMBasis = parent.is_valid() ? parent.getGlobalMBasis().copy() : new CartesianBasis(origin);
// 		localMBasis = parent.is_valid() ? parent.getLocalMBasis().copy() : new CartesianBasis(origin);
// 		globalMBasis.setIdentity();
// 		localMBasis.setIdentity();
// 	}
// 	if (parent == nullptr) {
// 		areGlobal = true;
// 	}
// }

// DMIKNode3D::DMIKNode3D(DMIKTransform p_globalBasis, Ref<DMIKNode3D> p_parent) {
// 	globalMBasis = p_globalBasis;
// 	createTempVars(globalBasis.origin);
// 	if (getParentAxes().is_valid())
// 		setParent(p_parent);
// 	else {
// 		areGlobal = true;
// 		localMBasis = p_globalBasis;
// 	}
// 	updateGlobal();
// }

// Vector3 DMIKNode3D::origin_() {
// 	updateGlobal();
// 	tempOrigin.set(getGlobalMBasis().origin);
// 	return tempOrigin;
// }

// Ref<DMIKNode3D> DMIKNode3D::getGlobalCopy() {
// 	return this->duplicate();
// }

// int DMIKNode3D::getGlobalChirality() {
// 	updateGlobal();
// 	return getGlobalMBasis().chirality;
// }

// int DMIKNode3D::getLocalChirality() {
// 	updateGlobal();
// 	return getLocalMBasis().chirality;
// }

// bool DMIKNode3D::isGlobalAxisFlipped(int axis) {
// 	updateGlobal();
// 	return globalMBasis.isAxisFlipped(axis);
// }

// bool DMIKNode3D::isLocalAxisFlipped(int axis) {
// 	return localMBasis.isAxisFlipped(axis);
// }

// void DMIKNode3D::setRelativeToParent(Ref<DMIKNode3D> par) {
// 	if (getParentAxes() != nullptr)
// 		getParentAxes().disown(this);
// 	parent = par;
// 	areGlobal = false;
// 	getParentAxes().registerDependent(this);
// 	markDirty();
// }

// bool DMIKNode3D::needsUpdate() {
// 	if (dirty)
// 		return true;
// 	else
// 		return false;
// }

// Ray DMIKNode3D::getGlobalOf(Ray in) {
// 	return new Ray(getGlobalOf(in.position), getGlobalOf(in.normal));
// }

// Vector3 DMIKNode3D::getGlobalOf(Vector3 in) {
// 	Vector3 result = in;
// 	setToGlobalOf(in, result);
// 	return result;
// }

// Ray DMIKNode3D::getLocalOf(Ray in) {
// 	Ray result = in;
// 	result.position = getLocalOf(in.position);
// 	result.normal = getLocalOf(in.normal);
// 	return result;
// }

// Vector3 DMIKNode3D::getLocalOf(Vector3 in) {
// 	updateGlobal();
// 	return getGlobalMBasis().getLocalOf(in);
// }

// void DMIKNode3D::translateByLocal(Vector3 translate) {
// 	updateGlobal();
// 	getLocalMBasis().translateBy(translate);
// 	markDirty();
// }

// void DMIKNode3D::translateByGlobal(Vector3 translate) {
// 	if (getParentAxes() != nullptr) {
// 		updateGlobal();
// 		translateTo(translate.addCopy(origin_()));
// 	} else {
// 		getLocalMBasis().translateBy(translate);
// 	}

// 	markDirty();
// }

// void DMIKNode3D::translateTo(Vector3 translate) {
// 	if (getParentAxes() != nullptr) {
// 		updateGlobal();
// 		getLocalMBasis().translateTo(getParentAxes().getGlobalMBasis().getLocalOf(translate));
// 		markDirty();
// 	} else {
// 		updateGlobal();
// 		getLocalMBasis().translateTo(translate);
// 		markDirty();
// 	}
// }

// void DMIKNode3D::translateTo(Vector3 translate, bool slip) {
// 	updateGlobal();
// 	if (slip) {
// 		Ref<DMIKNode3D> tempAbstractAxes = getGlobalCopy();
// 		tempAbstractAxes.translateTo(translate);
// 		slipTo(tempAbstractAxes);
// 	} else {
// 		translateTo(translate);
// 	}
// }

// void DMIKNode3D::setSlipType(int type) {
// 	if (getParentAxes() != nullptr) {
// 		if (type == IGNORE) {
// 			getParentAxes().dependentsRegistry.remove(this);
// 		} else if (type == NORMAL || type == FORWARD) {
// 			getParentAxes().registerDependent(this);
// 		}
// 	}
// 	slipType = type;
// }

// int DMIKNode3D::getSlipType() {
// 	return slipType;
// }

// void DMIKNode3D::rotateAboutX(float angle, bool orthonormalized) {
// 	updateGlobal();
// 	Quat xRot = new Quat(getGlobalMBasis().getXHeading(), angle);
// 	rotateBy(xRot);
// 	markDirty();
// }

// void DMIKNode3D::rotateAboutY(float angle, bool orthonormalized) {
// 	updateGlobal();
// 	Quat yRot = new Quat(getGlobalMBasis().getYHeading(), angle);
// 	rotateBy(yRot);
// 	markDirty();
// }

// void DMIKNode3D::rotateAboutZ(float angle, bool orthonormalized) {
// 	updateGlobal();
// 	Quat zRot = new Quat(getGlobalMBasis().getZHeading(), angle);
// 	rotateBy(zRot);
// 	markDirty();
// }

// void DMIKNode3D::rotateBy(Quat apply) {
// 	updateGlobal();
// 	if (getParentAxes() != nullptr) {
// 		Quat newRot = getParentAxes().getGlobalMBasis().getLocalOfRotation(apply);
// 		getLocalMBasis().rotateBy(newRot);
// 	} else {
// 		getLocalMBasis().rotateBy(apply);
// 	}

// 	markDirty();
// }

// void DMIKNode3D::rotateBy(Quat apply) {
// 	updateGlobal();
// 	if (parent != nullptr) {
// 		Quat newRot = getParentAxes().getGlobalMBasis().getLocalOfRotation(new Quat(apply));
// 		getLocalMBasis().rotateBy(newRot);
// 	} else {
// 		getLocalMBasis().rotateBy(new Quat(apply));
// 	}
// 	markDirty();
// }

// void DMIKNode3D::rotateByLocal(Quat apply) {
// 	updateGlobal();
// 	if (parent != nullptr) {
// 		getLocalMBasis().rotateBy(apply);
// 	}
// 	markDirty();
// }

// void DMIKNode3D::alignLocalsTo(Ref<DMIKNode3D> targetAxes) {
// 	getLocalMBasis().adoptValues(targetAxes.localMBasis);
// 	markDirty();
// }

// void DMIKNode3D::alignToParent() {
// 	getLocalMBasis().setIdentity();
// 	markDirty();
// }

// void DMIKNode3D::rotateToParent() {
// 	getLocalMBasis().rotateTo(new Quat(Quat.IDENTITY));
// 	markDirty();
// }

// void DMIKNode3D::alignGlobalsTo(Ref<DMIKNode3D> targetAxes) {
// 	targetAxes.updateGlobal();
// 	updateGlobal();
// 	if (getParentAxes() != nullptr) {
// 		getParentAxes().getGlobalMBasis().setToLocalOf(targetAxes.globalMBasis, localMBasis);
// 	} else {
// 		getLocalMBasis().adoptValues(targetAxes.globalMBasis);
// 	}
// 	markDirty();
// 	updateGlobal();
// }

// void DMIKNode3D::alignOrientationTo(Ref<DMIKNode3D> targetAxes) {
// 	targetAxes->updateGlobal();
// 	updateGlobal();
// 	if (getParentAxes() != nullptr) {
// 		getGlobalMBasis().rotateTo(targetAxes.getGlobalMBasis().rotation);
// 		getParentAxes().getGlobalMBasis().setToLocalOf(globalMBasis, localMBasis);
// 	} else {
// 		getLocalMBasis().rotateTo(targetAxes.getGlobalMBasis().rotation);
// 	}
// 	markDirty();
// }

// void DMIKNode3D::setGlobalOrientationTo(Quat rotation) {
// 	updateGlobal();
// 	if (getParentAxes() != nullptr) {
// 		getGlobalMBasis().rotateTo(rotation);
// 		getParentAxes().getGlobalMBasis().setToLocalOf(globalMBasis, localMBasis);
// 	} else {
// 		getLocalMBasis().rotateTo(rotation);
// 	}
// 	markDirty();
// }

// void DMIKNode3D::registerDependent(Ref<AxisDependency> newDependent) {
// 	//Make sure we don't hit a dependency loop
// 	if (DMIKNode3D.class.isAssignableFrom(newDependent.getClass())) {
// 		if (((DMIKNode3D)newDependent).isAncestorOf(this)) {
// 			transferToParent(((Ref<AxisDependency>)newDependent).getParentAxes());
// 		}
// 	}
// 	if (dependentsRegistry.indexOf(newDependent) == -1) {
// 		dependentsRegistry.add(new WeakRef<Ref<AxisDependency>>(newDependent));
// 	}
// }

// bool DMIKNode3D::isAncestorOf(Ref<DMIKNode3D> potentialDescendent) {
// 	bool result = false;
// 	Ref<DMIKNode3D> cursor = potentialDescendent.getParentAxes();
// 	while (cursor != nullptr) {
// 		if (cursor == this) {
// 			result = true;
// 			break;
// 		} else {
// 			cursor = cursor.getParentAxes();
// 		}
// 	}
// 	return result;
// }

// void DMIKNode3D::transferToParent(Ref<DMIKNode3D> newParent) {
// 	emancipate();
// 	setParent(newParent);
// }

// void DMIKNode3D::emancipate() {
// 	if (getParentAxes() != nullptr) {
// 		updateGlobal();
// 		Ref<DMIKNode3D> oldParent = getParentAxes();
// 		for (int32_t i = 0; i < dependentsRegistry.size(); i++) {
// 			dependentsRegistry[i]->get().parentChangeWarning(this, getParentAxes(), nullptr, nullptr);
// 		}
// 		getLocalMBasis().adoptValues(globalMBasis);
// 		getParentAxes().disown(this);
// 		parent = nullptr;
// 		areGlobal = true;
// 		markDirty();
// 		updateGlobal();
// 		for (WeakRef<Ref<AxisDependency>> ad : dependentsRegistry) {
// 			ad.get().parentChangeCompletionNotice(this, oldParent, nullptr, nullptr);
// 		}
// 	}
// }

// void DMIKNode3D::disown(Ref<AxisDependency> child) {
// 	dependentsRegistry.remove(child);
// }

// DMIKTransform DMIKNode3D::getGlobalMBasis() {
// 	updateGlobal();
// 	return globalMBasis;
// }

// DMIKTransform DMIKNode3D::getLocalMBasis() {
// 	return localMBasis;
// }

// void DMIKNode3D::axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis) {
// }

// void DMIKNode3D::axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis, List<Object> dontWarn) {
// 	updateGlobal();
// 	if (slipType == NORMAL) {
// 		if (getParentAxes() != nullptr) {
// 			Ref<DMIKNode3D> globalVals = relativeTo(globalPriorToSlipping);
// 			globalVals = globalPriorToSlipping.getLocalOf(globalVals);
// 			getLocalMBasis().adoptValues(globalMBasis);
// 			markDirty();
// 		}
// 	} else if (slipType == FORWARD) {
// 		Ref<DMIKNode3D> globalAfterVals = relativeTo(globalAfterSlipping);
// 		notifyDependentsOfSlip(globalAfterVals, dontWarn);
// 	}
// }

// void DMIKNode3D::axisSlipCompletionNotice(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> thisAxis) {
// }

// void DMIKNode3D::slipTo(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn) {
// 	updateGlobal();
// 	Ref<DMIKNode3D> originalGlobal = getGlobalCopy();
// 	notifyDependentsOfSlip(newAxisGlobal, dontWarn);
// 	Ref<DMIKNode3D> newVals = newAxisGlobal.getGlobalCopy();

// 	if (getParentAxes() != nullptr) {
// 		newVals = getParentAxes().getLocalOf(newAxisGlobal);
// 	}
// 	alignGlobalsTo(newAxisGlobal);
// 	markDirty();
// 	updateGlobal();

// 	notifyDependentsOfSlipCompletion(originalGlobal, dontWarn);
// }

// void DMIKNode3D::slipTo(Ref<DMIKNode3D> newAxisGlobal) {
// 	updateGlobal();
// 	Ref<DMIKNode3D> originalGlobal = getGlobalCopy();
// 	notifyDependentsOfSlip(newAxisGlobal);
// 	Ref<DMIKNode3D> newVals = newAxisGlobal.freeCopy();

// 	if (getParentAxes() != nullptr) {
// 		newVals = getParentAxes().getLocalOf(newVals);
// 	}
// 	getLocalMBasis().adoptValues(newVals.globalMBasis);
// 	dirty = true;
// 	updateGlobal();

// 	notifyDependentsOfSlipCompletion(originalGlobal);
// }

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
