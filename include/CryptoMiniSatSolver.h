#ifndef CM_SATSOLVER_H
#define CM_SATSOLVER_H

#ifdef SAT_CMSAT

#include "SATSolver.h"
#include <cryptominisat.h>

const int SAT_V = 10;
const int UNSAT_V = 20;

class CryptoMiniSatSolver : public SATSolver {

private:
	CMSat::SATSolver solver;
	std::vector<CMSat::Lit> assumptions;
	int32_t decision_vars;

public:
	CryptoMiniSatSolver(int32_t n_vars, int32_t n_args);
	~CryptoMiniSatSolver() {};
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