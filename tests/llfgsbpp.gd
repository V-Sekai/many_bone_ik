@tool
extends EditorScript

class Rosenbrock:
    extends LBFGSBSolver
    var n : int

    func _init(n_: int):
        self.n = n_

    func _call_operator(x: Array, grad: Array) -> float:
        var fx : float = pow((x[0] - 1.0), 2)
        grad[0] = 2 * (x[0] - 1) + 16 * (pow(x[0], 2) - x[1]) * x[0]
        
        for i in range(1, self.n):
            fx += 4 * pow((x[i] - pow(x[i - 1], 2)), 2)
            
            if(i == self.n - 1):
                grad[i] = 8 * (x[i] - pow(x[i - 1], 2))
            else:
                grad[i] = 8 * (x[i] - pow(x[i - 1], 2)) + 16 * (pow(x[i], 2) - x[i + 1]) * x[i]
                
        return fx


func _run():
    var n : int = 25
    var rosenbrock = Rosenbrock.new(n)
    
    var lb : Array[float] = []
    var ub : Array[float] = []
    var x : Array[float] = []
    
    for i in range(n):
        lb.append(2.0)
        ub.append(4.0)
        x.append(3.0)
    
    lb[2] = -INF
    ub[2] = INF
    
    x[0] = 2.0
    x[1] = 2.0
    x[5] = 4.0
    x[7] = 4.0
    
    var res: Array = rosenbrock.minimize(x, 0, lb, ub)
    
    print(str(n) + " iterations")
    print("x = " + str(res[0]))
    print("f(x) = " + str(res[1]))
    print("fx = " + str(res[2]))

# Output

# 25 iterations
# x = 13
# f(x) = [2, 2, 1.64742666556789, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2.10909336539027, 4]
# fx = 360.283585551151

# Example

# 13 iterations
# x = 2 2 1.64743 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2.10909 4
# f(x) = 360.284
# grad = 66 59.2823 8.76499e-07 58.2879 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48   44.509 2.23559e-06  -3.5862
# projected grad norm = 2.23559e-06
