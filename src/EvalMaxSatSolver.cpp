#ifdef SAT_EVALMAXSAT

#include "EvalMaxSatSolver.h"


EvalMaxSatSolver::EvalMaxSatSolver(int32_t n_vars, int32_t n_args) {
	decision_vars = n_vars;
    for (uint32_t i = 0; i < n_vars; i++) {
        solver.newVar();
    }
	model = std::vector<bool>(n_vars);
}

void EvalMaxSatSolver::add_hard_clause(const std::vector<int32_t> & clause) {
	solver.addClause(clause);
}

void EvalMaxSatSolver::add_soft_clause(const std::vector<int32_t> & clause) {
	solver.addClause(clause, 1);
}

int EvalMaxSatSolver::solve() {
	bool sat = solver.solve();
    if (!sat) {
        return 20;
    } else {
        model.clear();
		for (int32_t i = 0; i < decision_vars; i++)
			model[i] = solver.getValue(i+1);
	}
	return sat;
}
#endif