#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include <vector>
#include <cstdint>


class SATSolver {

public:
	SATSolver() {}
	virtual ~SATSolver() {}
	virtual void add_clause(const std::vector<int32_t> & clause) = 0;
	virtual void assume(int32_t lit) = 0;
	virtual int solve() = 0;
	virtual int solve(const std::vector<int32_t> assumptions) = 0;
};
#endif
