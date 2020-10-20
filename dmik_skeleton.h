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
private:
	Vector3 xBase = Vector3(1, 0, 0);
	Vector3 yBase = Vector3(0, 1, 0);
	Vector3 zBase = Vector3(0, 0, 1);
	Ray xRay = Ray(Vector3(0, 0, 0), Vector3(1, 0, 0));
	Ray yRay = Ray(Vector3(0, 0, 0), Vector3(0, 1, 0));
	Ray zRay = Ray(Vector3(0, 0, 0), Vector3(0, 0, 1));
	/**
	 * a vector representing the translation of this basis relative to its parent. 
	 */
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
	int LEFT = -1;
	int RIGHT = 1;
	int chirality = RIGHT;
	int NONE = -1;
	int X = 0;
	int Y = 1;
	int Z = 2;

	Quat rotation;
	Quat inverseRotation;
	Vector3 translate;

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

	DMIKTransform() {
		Vector3 origin;
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

	DMIKTransform(const Transform &input) {
		translate = input.origin;
		Vector3 x = input.basis.get_axis(0);
		Vector3 y = input.basis.get_axis(1);
		Vector3 z = input.basis.get_axis(2);
		set(x, y, z);
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

	void setToLocalOf(DMIKTransform global_input, DMIKTransform &local_output) {
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
	void setToGlobalOf(DMIKTransform localInput, DMIKTransform &globalOutput) {
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

class DMIKNode3D : public AxisDependency {
private:
	Ref<AxisDependency> parent;
	int slipType = 0;

protected:
	Vector3 workingVector;
	bool areGlobal = true;

public:
	int NORMAL = 0, IGNORE = 1, FORWARD = 2;
	int RIGHT = 1, LEFT = -1;
	int X = 0, Y = 1, Z = 2;
	bool debug = false;
	DMIKTransform localMBasis;
	DMIKTransform globalMBasis;
	bool dirty = true;

	Vector3 tempOrigin;

	List<Ref<AxisDependency>> dependentsRegistry;
	void createTempVars(Vector3 type);

	Ref<AxisDependency> getParentAxes();

	void updateGlobal();

	/**
     * Sets the parentAxes for this axis globally.
     * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz
     * change.
     *
     *   @param intendedParent the new parent Axes
     *   @param requestedBy the object making thisRequest, will be passed on to parentChangeWarning
     *   for any AxisDependancy objects registered with this Ref<AbstractAxes>  (can be nullptr if not important)
     **/
	void setParent(Ref<DMIKNode3D> intendedParent, Ref<DMIKNode3D> requestedBy);

	/**
     * Sets the parentAxes for this axis globally.
     * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz
     * change.
     *
     *   @param par the new parent Axes
     **/
	void setParent(Ref<DMIKNode3D> par);

	Vector3 origin_();

	/**
		 * Make a GlobalCopy of these Axes.
		 * @return
		 */
	Ref<DMIKNode3D> getGlobalCopy();

	int getGlobalChirality();

	int getLocalChirality();

	/**
		 * True if the input axis of this Axes object in global coordinates should be multiplied by negative one after rotation.
		 * By default, this always returns false. But can be overridden for more advanced implementations
		 * allowing for reflection transformations.
		 * @param axis
		 * @return true if axis should be flipped, false otherwise. Default is false.
		 */
	bool isGlobalAxisFlipped(int axis);

	/**
		 * True if the input axis of this Axes object in local coordinates should be multiplied by negative one after rotation.
		 * By default, this always returns false. But can be overridden for more advanced implementations
		 * allowing for reflection transformations.
		 * @param axis
		 * @return true if axis should be flipped, false otherwise. Default is false.
		 */
	bool isLocalAxisFlipped(int axis);

	/**
		 * Sets the parentAxes for this axis locally.
		 * in other words, lx, ly, lz remain unchanged, but globalX, globalY, and globalZ
		 * change.
		 *
		 * if setting this parent would result in a dependency loop, then the input Axes
		 * parent is set to this Axes' parent, prior to this axes setting the input axes
		 * as its parent.
		 **/
	void setRelativeToParent(Ref<DMIKNode3D> par);

	bool needsUpdate();

	/**
		 * Given a vector in this axes local coordinates, returns the vector's position in global coordinates.
		 * @param in
		 * @return
		 */
	Vector3 getGlobalOf(Vector3 in);

	/**
		 *  Given a vector in this axes local coordinates, modifies the vector's values to represent its position global coordinates.
		 * @param in
		 * @return a reference to this the @param in object.
		 */
	Vector3 setToGlobalOf(
			Vector3 in);

	/**
		 *  Given an input vector in this axes local coordinates, modifies the output vector's values to represent the input's position in global coordinates.
		 * @param in
		 */
	void setToGlobalOf(Vector3 input, Vector3 &output);

	/**
		 *  Given an input sgRay in this axes local coordinates, modifies the output Rays's values to represent the input's in global coordinates.
		 * @param in
		 */
	void setToGlobalOf(Ray input, Ray &output);

	Ray getGlobalOf(Ray in);

	/**
		 * returns an axis representing the global location of this axis if the input axis were its parent.
		 * @param in
		 * @return
		 */
	virtual Ref<DMIKNode3D> relativeTo(Ref<DMIKNode3D> in) { return Ref<DMIKNode3D>(); }

	Vector3 getLocalOf(Vector3 in);

	/**
		 *  Given a vector in global coordinates, modifies the vector's values to represent its position in theseAxes local coordinates.
		 * @param in
		 * @return a reference to the @param in object.
		 */
	Vector3 setToLocalOf(Vector3 in);

	/**
		 *  Given a vector in global coordinates, modifies the vector's values to represent its position in theseAxes local coordinates.
		 * @param in
		 */
	void setToLocalOf(Vector3 in, Vector3 &out);

	/**
		 *  Given a sgRay in global coordinates, modifies the sgRay's values to represent its position in theseAxes local coordinates.
		 * @param in
		 */
	void setToLocalOf(Ray in, Ray &out);
	void setToLocalOf(DMIKTransform input, DMIKTransform &output);

	void setToLocalOf(DMIKNode3D input, DMIKNode3D &output);

	Ray getLocalOf(Ray in);
	virtual Ref<DMIKNode3D> getLocalOf(Ref<DMIKNode3D> input) { return Ref<DMIKNode3D>(); };

	virtual DMIKTransform getLocalOf(DMIKNode3D input) { return DMIKTransform(); };

	void translateByLocal(Vector3 translate);
	void translateByGlobal(Vector3 translate);
	void translateTo(Vector3 translate, bool slip);

	void translateTo(Vector3 translate);

	/**
		 * @return a copy of these axes that does not refer to any parent.
		 * Because there is no parent, the copy's global coordinates and local coordinates will be equivalent to each other.
		 * The copy's local coordinates will also be equivalent to the original's local coordinates. However, the copy's
		 * global coordinates will likely be drastically different from the original's global coordinates.
		 *
		 */
	virtual Ref<DMIKNode3D> freeCopy() { return Ref<DMIKNode3D>(); };

	/**
		 *  return a ray / segment representing this Axes global x basis position and direction and magnitude
		 * @return a ray / segment representing this Axes global x basis position and direction and magnitude
		 */
	virtual Ray x_() { return Ray(); };

	/**
		 *  return a ray / segment representing this Axes global y basis position and direction and magnitude
		 * @return a ray / segment representing this Axes global y basis position and direction and magnitude
		 */
	virtual Ray y_() { return Ray(); };

	/**
		 *  return a ray / segment representing this Axes global z basis position and direction and magnitude
		 * @return a ray / segment representing this Axes global z basis position and direction and magnitude
		 */
	virtual Ray z_() { return Ray(); };

	/**
	 * Creates an exact copy of this Axes object. Attached to the same parent as this Axes object
	 * @param slipAware
	 * @return
	 */
	virtual Ref<DMIKNode3D> attachedCopy(bool slipAware){ return Ref<DMIKNode3D>(); };
	void setSlipType(int type);
	int getSlipType();
	void rotateAboutX(float angle, bool orthonormalized);
	void rotateAboutY(float angle, bool orthonormalized);
	void rotateAboutZ(float angle, bool orthonormalized);

	/**
		 * Rotates the bases around their origin in global coordinates
		 * @param rotation
		 */
	void rotateBy(Quat apply);

	/**
		 * rotates the bases around their origin in Local coordinates
		 * @param rotation
		 */
	void rotateByLocal(Quat apply);

	/**
		 * sets these axes to have the same orientation and location relative to their parent
		 * axes as the input's axes do to the input's parent axes.
		 *
		 * If the axes on which this function is called are orthonormal,
		 * this function normalizes and orthogonalizes them regardless of whether the targetAxes are orthonormal.
		 *
		 * @param targetAxes the Axes to make this Axis identical to
		 */
	void alignLocalsTo(Ref<DMIKNode3D> targetAxes);

	/**
		 * sets the bases to the Identity basis and Identity rotation relative to its parent, and translates
		 * its origin to the parent's origin.
		 *
		 * be careful calling this method, as it destroys any shear / scale information.
		 */
	void alignToParent();

	/**
		 * rotates and translates the axes back to its parent, but maintains
		 * its shear, translate and scale attributes.
		 */
	void rotateToParent();

	/**
		 * sets these axes to have the same global orientation as the input Axes.
		 * these Axes lx, ly, and lz headings will differ from the target ages,
		 * but its gx, gy, and gz headings should be identical unless this
		 * axis is orthonormalized and the target axes are not.
		 * @param targetAxes
		 */
	void alignGlobalsTo(Ref<DMIKNode3D> targetAxes);
	void alignOrientationTo(Ref<DMIKNode3D> targetAxes);

	/**
		 * updates the axes object such that its global orientation
		 * matches the given Quat object.
		 * @param rotation
		 */
	void setGlobalOrientationTo(Quat rotation);
	void registerDependent(Ref<AxisDependency> newDependent);
	bool isAncestorOf(Ref<DMIKNode3D> potentialDescendent);

	/**
		 * unregisters this Ref<AbstractAxes> from its current parent and
		 * registers it to a new parent without changing its global position or orientation
		 * when doing so.
		 * @param newParent
		 */

	void transferToParent(Ref<DMIKNode3D> newParent);

	/**
		 * unregisters this Ref<AbstractAxes> from its parent,
		 * but keeps its global position the same.
		 */
	void emancipate();
	void disown(Ref<AxisDependency> child);
	DMIKTransform getGlobalMBasis();
	DMIKTransform getLocalMBasis();
	void axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis, List<Object> dontWarn);

	/**
		 * if the input axes have have the same global
		 * values as these axes, returns true, otherwise, returns false.
		 *
		 * This function is orthonormality aware. Meaning, if the orthonormality
		 * constraint is enabled on either axes, that axes' orthonormal version
		 * will be used in the comparison.
		 * @param ax
		 */
	bool equals(Ref<DMIKNode3D> ax);
	void axisSlipWarning(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> actualAxis);
	void axisSlipCompletionNotice(Ref<DMIKNode3D> globalPriorToSlipping, Ref<DMIKNode3D> globalAfterSlipping, Ref<DMIKNode3D> thisAxis);
	void slipTo(Ref<DMIKNode3D> newAxisGlobal);
	void slipTo(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn);
	// void notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn);
	// void notifyDependentsOfSlipCompletion(Ref<DMIKNode3D> globalAxisPriorToSlipping, List<Object> dontWarn) {
	// 	for (int i = 0; i < dependentsRegistry.size(); i++) {
	// 		List<Ref<AxisDependency>>::Element *O = dontWarn.find(dependentsRegistry[i]);
	// 		if (O) {
	// 			O->get().axisSlipCompletionNotice(globalAxisPriorToSlipping, getGlobalCopy(), this);
	// 		}
	// 	}
	// }
	// void notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal);
	// void notifyDependentsOfSlipCompletion(Ref<DMIKNode3D> globalAxisPriorToSlipping) {
	// 	for (int i = 0; i < dependentsRegistry.size(); i++) {
	// 		dependentsRegistry[i]->axisSlipCompletionNotice(globalAxisPriorToSlipping, getGlobalCopy(), this);
	// 	}
	// }

	void markDirty() {
		if (!dirty) {
			dirty = true;
			markDependentsDirty();
		}
	}

	void markDependentsDirty() {
		List<Ref<AxisDependency>>::Element *i = dependentsRegistry.front();
		while (i) {
			Ref<AxisDependency> dr = i->get();
			if (dr.is_valid()) {
				dr->markDirty();
			}
			i = i->next();
		}
	}
	DMIKNode3D();
	/**
     * @param globalMBasis a Basis object for this Axes to adopt the values of
     * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
     */
	DMIKNode3D(DMIKTransform p_globalBasis, Ref<DMIKNode3D> p_parent);

	/**
		 * @param origin the center of this axes basis. The basis vector parameters will be automatically ADDED to the origin in order to create this basis vector.
		 * @param inX the direction of the X basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
		 * @param inY the direction of the Y basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
		 * @param inZ the direction of the Z basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
		 * @param forceOrthoNormality
		 * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
		 */
	DMIKNode3D(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, Ref<DMIKNode3D> parent, bool customBases);
		
	DMIKNode3D(Transform p_globalBasis, Ref<DMIKNode3D> p_parent) {
		globalMBasis = p_globalBasis;
		createTempVars(p_globalBasis.origin);
		if (getParentAxes().is_valid()) {
			setParent(p_parent);
		} else {
			areGlobal = true;
			localMBasis = p_globalBasis;
		}
		updateGlobal();
	}
};

