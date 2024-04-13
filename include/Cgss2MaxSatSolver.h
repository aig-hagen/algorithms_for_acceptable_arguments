#ifndef CGSS2SOLVER_H
#define CGSS2SOLVER_H

#ifdef SAT_CGSS2

#include "SATSolver.h"
#include <cgss2.h>


const int ERROR_V = 0;
const int SAT_V = 30;
const int UNSAT_V = 20;


using namespace cgss2;


class Cgss2MaxSatSolver : public SATSolver {

private:
	CGSS2 * solver;
	std::vector<int32_t> assumptions;
	int32_t decision_vars;

public:
	Cgss2MaxSatSolver(int32_t n_vars, int32_t n_args);
	~Cgss2MaxSatSolver() { delete solver; };
	void add_clause(const std::vector<int32_t> & clause);
	void add_clause_1(int32_t lit);
	void add_clause_2(int32_t lit1, int32_t lit2);
	void add_clause_3(int32_t lit1, int32_t lit2, int32_t lit3);
    void add_soft_constraint(int32_t lit);
    void disable_soft_constraint(int32_t lit);
	void assume(int32_t lit);
	int solve();
	int solve(const std::vector<int32_t> assumptions);
	uint64_t get_optimal_cost();
	std::vector<bool> model;
};
#endif
#endif