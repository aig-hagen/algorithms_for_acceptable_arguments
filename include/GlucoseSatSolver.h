#ifndef GLUCOSESATSOLVER_H
#define GLUCOSESATSOLVER_H

#ifdef SAT_GLUCOSE

#include "SATSolver.h"
#include <core/Solver.h>

const int SAT_V = 10;
const int UNSAT_V = 20;

class GlucoseSatSolver : public SATSolver {

private:
	Glucose::Solver * solver;
	Glucose::vec<Glucose::Lit> assumptions;
	int32_t decision_vars;

public:
	GlucoseSatSolver(int32_t n_vars, int32_t n_args);
	~GlucoseSatSolver() { delete solver; };
	void add_clause(const std::vector<int32_t> & clause);
	void add_clause_1(int32_t lit);
	void add_clause_2(int32_t lit1, int32_t lit2);
	void assume(int32_t lit);
	int solve();
	int solve(const std::vector<int32_t> assumptions);
	std::vector<bool> model;
};
#endif
#endif
