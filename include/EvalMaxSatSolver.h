#ifndef EVALMAXSAT_H
#define EVALMAXSAT_H

#ifdef SAT_EVALMAXSAT

#ifndef EMS_H
#define EMS_H
#include <lib/EvalMaxSAT/src/EvalMaxSAT.h>
#endif


class EvalMaxSatSolver {

private:
	EvalMaxSAT<> solver;
	int32_t decision_vars;

public:
	EvalMaxSatSolver(int32_t n_vars, int32_t n_args);
	~EvalMaxSatSolver() {};
	void add_clause(const std::vector<int32_t> & clause);
    void add_soft_clause(const std::vector<int32_t> & clause);
	int solve();
	std::vector<bool> model;

};

#endif
#endif