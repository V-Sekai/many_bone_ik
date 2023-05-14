package math.doubleV;

import math.floatV.Vec3f;

/**
 * Minimal implementation of basis transformations.
 * Supports only righthanded orthonormal bases (no scaling, now skewing, no
 * reflections).
 * 
 * @author Eron Gjoni
 *
 */
public abstract class IKBasis {
	public static final int LEFT = -1;
	public static final int RIGHT = 1;

	public int chirality = RIGHT;

	public static final int NONE = -1;
	public static final int X = 0;
	public static final int Y = 1;
	public static final int Z = 2;

	public Quaternion rotation = new Quaternion();
	public Quaternion inverseRotation = new Quaternion();
	/**
	 * a vector respresnting the translation of this basis relative to its parent.
	 */
	public Vector3 translate;

	protected Vector3 xBase;
	protected Vector3 yBase;
	protected Vector3 zBase;

	protected IKRay3D xRay;
	protected IKRay3D yRay;
	protected IKRay3D zRay;

	/**
	 * Initialize this basis at the origin. The basis will be righthanded by
	 * default.
	 * 
	 * @param origin
	 */
	public IKBasis(Vector3 origin) {
		translate = origin.copy();
		xBase = origin.copy();
		yBase = origin.copy();
		zBase = origin.copy();
		xBase.set(1, 0, 0);
		yBase.set(0, 1, 0);
		zBase.set(0, 0, 1);
		Vector3 zero = origin.copy();
		zero.set(0, 0, 0);
		xRay = new IKRay3D(zero.copy(), xBase.copy());
		yRay = new IKRay3D(zero.copy(), yBase.copy());
		zRay = new IKRay3D(zero.copy(), zBase.copy());
		refreshPrecomputed();
	}

	public <T extends IKBasis> IKBasis(T input) {
		translate = input.translate.copy();
		xBase = translate.copy();
		yBase = translate.copy();
		zBase = translate.copy();
		xBase.set(1, 0, 0);
		yBase.set(0, 1, 0);
		zBase.set(0, 0, 1);
		Vector3 zero = translate.copy();
		zero.set(0, 0, 0);
		xRay = new IKRay3D(zero.copy(), xBase.copy());
		yRay = new IKRay3D(zero.copy(), yBase.copy());
		zRay = new IKRay3D(zero.copy(), zBase.copy());
		this.adoptValues(input);

	}

	/**
	 * Initialize this basis at the origin.
	 * The basis will be backed by a rotation object which presumes right handed
	 * chirality.
	 * Therefore, the rotation object will align so its local XY plane aligns with
	 * this basis' XY plane
	 * Afterwards, it will check chirality, and if the basis isn't righthanded, this
	 * class will assume the
	 * z-axis is the one that's been flipped.
	 * 
	 * If you want to manually specify which axis has been flipped
	 * (so that the rotation object aligns with respect to the plane formed
	 * by the other two basis vectors) then use the constructor dedicated for that
	 * purpose
	 * 
	 * @param origin
	 * @param x      basis vector direction
	 * @param y      basis vector direction
	 * @param z      basis vector direction
	 */
	public IKBasis(Vector3 origin, Vector3 x, Vector3 y, Vector3 z) {
		this.translate = origin.copy();
		xRay = new IKRay3D(origin.copy(), origin.copy());
		yRay = new IKRay3D(origin.copy(), origin.copy());
		zRay = new IKRay3D(origin.copy(), origin.copy());
		this.set(x.copy(), y.copy(), z.copy());
	}

	/**
	 * Initialize this basis at the origin defined by the base of the @param x Ray.
	 * 
	 * The basis will be backed by a rotation object which presumes right handed
	 * chirality.
	 * Therefore, the rotation object will align so its local XY plane aligns with
	 * this basis' XY plane
	 * Afterwards, it will check chirality, and if the basis isn't righthanded, this
	 * class will assume the
	 * z-axis is the one that's been flipped.
	 * 
	 * If you want to manually specify which axis has been flipped
	 * (so that the rotation object aligns with respect to the plane formed
	 * by the other two basis vectors) then use the constructor dedicated for that
	 * purpose
	 * 
	 * 
	 * @param x basis Ray
	 * @param y basis Ray
	 * @param z basis Ray
	 */
	public <R extends IKRay3D> IKBasis(R x, R y, R z) {
		this.translate = x.p1().copy();
		xRay = x.copy();
		yRay = y.copy();
		zRay = z.copy();
		Vector3 xDirNew = x.heading().copy();
		Vector3 yDirNew = y.heading().copy();
		Vector3 zDirNew = z.heading().copy();
		xDirNew.normalize();
		yDirNew.normalize();
		zDirNew.normalize();
		set(xDirNew, yDirNew, zDirNew);
	}

	/**
	 * @param translation an array of THREE numbers corresponding to the translation
	 *                    of this transform in the space of its parent transform
	 * @param rotation    an array of FOUR numbers corresponding to the rotation of
	 *                    this transform in the space of its parent bone's
	 *                    transform.
	 *                    The four numbers should be a Hamilton quaternion
	 *                    representation (not JPL, important!), in the form [W, X,
	 *                    Y, Z], where W is the scalar quaternion component
	 * @param scale       an array of THREE numbers corresponding to scale of the X,
	 *                    Y, and Z components of this transform. The convention is
	 *                    that a value of [1,1,1] indicates
	 */
	public void set(double[] translation, double[] rotation, double[] scale) {
		this.translate.set(translation);
		this.xBase.set(scale[0], 0, 0);
		this.yBase.set(0, scale[1], 0);
		this.zBase.set(0, 0, scale[2]);
		this.rotation.rotation.set(rotation[0], rotation[1], rotation[2], rotation[3], true);
		this.refreshPrecomputed();
	}

	private void set(Vector3 x, Vector3 y, Vector3 z) {
		xBase = translate.copy();
		yBase = translate.copy();
		zBase = translate.copy();
		xBase.set(1, 0, 0);
		yBase.set(0, 1, 0);
		zBase.set(0, 0, 1);
		Vector3 zero = translate.copy();
		zero.set(0, 0, 0);
		xRay.setP1(zero.copy());
		xRay.setP2(xBase.copy());
		yRay.setP1(zero.copy());
		yRay.setP2(yBase.copy());
		zRay.setP1(zero.copy());
		zRay.setP2(zBase.copy());
		this.rotation = createPrioritzedRotation(x, y, z);
		this.refreshPrecomputed();
	}

	/**
	 * takes on the same values (not references) as the input basis.
	 * 
	 * @param in
	 */
	public <T extends IKBasis> void adoptValues(T in) {
		this.translate.set(in.translate);
		this.rotation.set(in.rotation);
		xBase = translate.copy();
		yBase = translate.copy();
		zBase = translate.copy();
		xBase.set(1, 0, 0);
		yBase.set(0, 1, 0);
		zBase.set(0, 0, 1);
		xRay = in.xRay.copy();
		yRay = in.yRay.copy();
		zRay = in.zRay.copy();
		this.refreshPrecomputed();
	}

	public void setIdentity() {
		this.translate.set(0, 0, 0);
		xBase.set(1, 0, 0);
		yBase.set(0, 1, 0);
		zBase.set(0, 0, 1);
		this.xRay.p1.set(this.translate);
		this.xRay.p2.set(xBase);
		this.yRay.p1.set(this.translate);
		this.yRay.p2.set(yBase);
		this.zRay.p1.set(this.translate);
		this.zRay.p2.set(zBase);
		this.rotation = new Quaternion();
		refreshPrecomputed();
	}

	private Quaternion createPrioritzedRotation(Vector3 xHeading, Vector3 yHeading, Vector3 zHeading) {

		Vector3 tempV = zHeading.copy();
		tempV.set(0, 0, 0);
		Quaternion toYZ = new Quaternion(yBase, zBase, yHeading, zHeading);
		toYZ.applyTo(yBase, tempV);
		Quaternion toY = new Quaternion(tempV, yHeading);

		return toY.applyTo(toYZ);
	}

	public Quaternion getLocalOfRotation(Quaternion inRot) {
		Quaternion resultNew = inverseRotation.applyTo(inRot).applyTo(rotation);
		return resultNew;
	}

	public <B extends IKBasis> void setToLocalOf(B global_input, B local_output) {
		local_output.translate = this.getLocalOf(global_input.translate);
		inverseRotation.applyTo(global_input.rotation, local_output.rotation);

		local_output.refreshPrecomputed();
	}

	public void refreshPrecomputed() {
		this.rotation.setToReversion(inverseRotation);
		this.updateRays();
	}

	public Vector3 getLocalOf(Vector3 v) {
		Vector3 result = (Vector3) v.copy();
		setToLocalOf(v, result);
		return result;
	}

	public void setToLocalOf(Vector3 input, Vector3 output) {
		output.set(input);
		output.sub(this.translate);
		inverseRotation.applyTo(output, output);
	}

	public void rotateTo(Quaternion newRotation) {
		this.rotation.set(newRotation);
		this.refreshPrecomputed();
	}

	public void rotateBy(Quaternion addRotation) {
		addRotation.applyTo(this.rotation, this.rotation);
		this.refreshPrecomputed();
	}

	/**
	 * the default Basis implementation is orthonormal,
	 * so by default this function will just set @param vec to (1,0,0),
	 * but extending (affine) classes can override this to represent the direction
	 * and magnitude of the x axis prior to rotation.
	 */
	public void setToShearXBase(Vector3 vec) {
		vec.set(xBase);
	}

	/**
	 * the default Basis implementation is orthonormal,
	 * so by default this function will just set @param vec to (0,1,0),
	 * but extending (affine) classes can override this to represent the direction
	 * and magnitude of the y axis prior to rotation.
	 */
	public void setToShearYBase(Vector3 vec) {
		vec.set(yBase);
	}

	/**
	 * the default Basis implementation is orthonormal,
	 * so by default this function will just set @param vec to (0,0,1),
	 * but extending (affine) classes can override this to represent the direction
	 * and magnitude of the z axis prior to rotation.
	 */
	public void setToShearZBase(Vector3 vec) {
		vec.set(zBase);
	}

	/**
	 * sets globalOutput such that the result of
	 * this.getLocalOf(globalOutput) == localInput.
	 * 
	 * @param localInput
	 * @param globalOutput
	 */
	public <T extends IKBasis> void setToGlobalOf(T localInput, T globalOutput) {
		this.rotation.applyTo(localInput.rotation, globalOutput.rotation);
		this.setToGlobalOf(localInput.translate, globalOutput.translate);
		globalOutput.refreshPrecomputed();
	}

	public void setToGlobalOf(Vector3 input, Vector3 output) {
		try {
			rotation.applyTo(input, output);
			output.add(this.translate);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void translateBy(Vector3 transBy) {
		this.translate.x += transBy.x;
		this.translate.y += transBy.y;
		this.translate.z += transBy.z;
		updateRays();
	}

	public void translateTo(Vector3 newOrigin) {
		this.translate.x = newOrigin.x;
		this.translate.y = newOrigin.y;
		this.translate.z = newOrigin.z;
		updateRays();
	}

	public IKRay3D getXRay() {
		return xRay;
	}

	public IKRay3D getYRay() {
		return yRay;
	}

	public IKRay3D getZRay() {
		return zRay;
	}

	public Vector3 getXHeading() {
		return this.xRay.heading();
	}

	public Vector3 getYHeading() {
		return this.yRay.heading();
	}

	public Vector3 getZHeading() {
		return this.zRay.heading();
	}

	public Vector3 getOrigin() {
		return translate;
	}

	/**
	 * true if the input axis should be multiplied by negative one after rotation.
	 * By default, this always returns false. But can be overriden for more advanced
	 * implementations
	 * allowing for reflection transformations.
	 * 
	 * @param axis
	 * @return true if axis should be flipped, false otherwise. Default is false.
	 */
	public boolean isAxisFlipped(int axis) {
		return false;
	}

	/**
	 * @return a precomputed inverse of the rotation represented by this basis
	 *         object.
	 */
	public Quaternion getInverseRotation() {
		return this.inverseRotation;
	}

	private void updateRays() {
		xRay.setP1(this.translate);
		xRay.p2.set(xBase);
		yRay.setP1(this.translate);
		yRay.p2.set(yBase);
		zRay.setP1(this.translate);
		zRay.p2.set(zBase);

		rotation.applyTo(xRay.p2, xRay.p2);
		rotation.applyTo(yRay.p2, yRay.p2);
		rotation.applyTo(zRay.p2, zRay.p2);

		xRay.p2.add(this.translate);
		yRay.p2.add(this.translate);
		zRay.p2.add(this.translate);
	}

	public abstract IKBasis copy();

	public String print() {
		Vec3f xh = xRay.heading().toVec3f();

		Vec3f yh = yRay.heading().toVec3f();

		Vec3f zh = zRay.heading().toVec3f();

		float xMag = xh.mag();
		float yMag = yh.mag();
		float zMag = zh.mag();
		String chirality = this.chirality == LEFT ? "LEFT" : "RIGHT";
		String result = "-----------\n"
				+ chirality + " handed \n"
				+ "origin: " + this.translate.toVec3f() + "\n"
				+ "rot Axis: " + this.rotation.getAxis().toVec3f() + ", "
				+ "Angle: " + (float) Math.toDegrees(this.rotation.getAngle()) + "\n"
				+ "xHead: " + xh + ", mag: " + xMag + "\n"
				+ "yHead: " + yh + ", mag: " + yMag + "\n"
				+ "zHead: " + zh + ", mag: " + zMag + "\n";

		return result;
	}

}