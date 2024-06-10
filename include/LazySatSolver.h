#ifndef LAZY_H
#define LAZY_H

#ifdef SAT_CADICAL

#include "CadicalSatSolver.h"

class LazySatSolver : public SATSolver {

private:
	CaDiCaL::Solver * solver;
	std::vector<int32_t> assumptions;
	int32_t decision_vars;
    std::vector<std::vector<int32_t>> clauses;
    uint32_t num_clauses;

public:
	LazySatSolver(int32_t n_vars, int32_t n_args);
	~LazySatSolver() { delete solver; };
	void add_clause(const std::vector<int32_t> & clause);
	void add_clause_1(int32_t lit);
	void add_clause_2(int32_t lit1, int32_t lit2);
	void add_clause_3(int32_t lit1, int32_t lit2, int32_t lit3);
    void add_temp_clause(const std::vector<int32_t> & clause);
	void add_temp_clause_1(int32_t lit);
	void add_temp_clause_2(int32_t lit1, int32_t lit2);
	void add_temp_clause_3(int32_t lit1, int32_t lit2, int32_t lit3);
	void assume(int32_t lit);
	int solve();
	int solve(const std::vector<int32_t> assumptions);
	std::vector<bool> model;
};
#endif
#endif