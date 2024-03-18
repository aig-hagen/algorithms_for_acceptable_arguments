#ifdef EVALMAX_SATSOLVER_H

#include "EvalMaxSAT.h"


class EvalMaxSatSolver {

private:
	EvalMaxSAT<> solver;
	int32_t decision_vars;

public:
	EvalMaxSatSolver(int32_t n_vars, int32_t n_args);
	~EvalMaxSatSolver() {};
	void add_hard_clause(const std::vector<int32_t> & clause);
    void add_soft_clause(const std::vector<int32_t> & clause);
	int solve();
	std::vector<bool> model;

};
#endif