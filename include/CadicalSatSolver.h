#ifndef CADICAL_H
#define CADICAL_H

#ifdef SAT_CADICAL

#include "SATSolver.h"
#include <cadical.hpp>

const int ERROR_V = 0;
const int SAT_V = 10;
const int UNSAT_V = 20;

class CadicalSatSolver : public SATSolver {

private:
	CaDiCaL::Solver * solver;
	std::vector<int32_t> assumptions;
	int32_t decision_vars;

public:
	CadicalSatSolver(int32_t n_vars, int32_t n_args);
	~CadicalSatSolver() { delete solver; };
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