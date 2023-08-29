@tool
extends EditorScript

class Quadratic:
	extends LBFGSBSolver
	func _call_operator(x : Array[float], grad : Array[float]) -> float:
		var n : int = x.size()
		var d : Array = []
		
		for i in range(n):
			d.append(i)
			
		var f : float = 0.0
		for i in range(n):
			f += pow((x[i] - d[i]), 2)
			
		for i in range(n):
			grad[i] = 2.0 * (x[i] - d[i])
			
		return f


class Rosenbrock:
	extends LBFGSBSolver
	var n : int

	func _init(n_: int):
		self.n = n_

	func call_operator(x: Array, grad: Array) -> float:
		var fx : float = 0.0
		for i in range(0, self.n, 2):
			var t1 : float = 1.0 - x[i]
			var t2 : float = 10 * (x[i + 1] - x[i] * x[i])
			grad[i + 1] = 20 * t2
			grad[i] = -2.0 * (x[i] * grad[i + 1] + t1)
			fx += t1 * t1 + t2 * t2
		return fx

func _run():
	var lb : Array[float] = []
	var ub : Array[float] = []
	var x : Array[float] = []
	var n : int = 10

	lb.resize(n)
	lb.fill(-100)
	ub.resize(n)
	ub.fill(100)
	x.resize(n)
	x.fill(0)

	var fx : float = 0
	var solver = Quadratic.new()
	var niter : int = 0
	var ret: Array = solver.minimize(x, fx, lb, ub)
	niter = ret[0]
	x = ret[1]
	fx = ret[2]

	print(str(niter) + " iterations")
	print("x = " + str(x))
	print("f(x) = " + str(fx))

	lb.fill(2)
	x.fill(3)
	ub.fill(4)
	solver = Rosenbrock.new(n)
	fx = 0
	ret = solver.minimize(x, fx, lb, ub)
	niter = ret[0]
	x = ret[1]
	fx = ret[2]
	print(str(niter) + " iterations")
	print("x = " + str(x))
	print("f(x) = " + str(fx))

	return 0;
