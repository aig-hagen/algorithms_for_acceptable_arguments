#ifdef SAT_KISSAT

#include "KissatSolver.h"

#include <iostream>

KissatSolver::KissatSolver(int32_t n_vars, int32_t n_args) {
    solver = kissat_init();
    kissat_reserve(solver, n_vars);
	decision_vars = n_args;
	model = std::vector<bool>(decision_vars);
}

void KissatSolver::add_clause(const std::vector<int32_t> & clause) {
	for (int32_t i : clause) {
		kissat_add(solver, i);
	}
	kissat_add(solver, 0);
}

void KissatSolver::add_clause_1(int32_t lit) {
	kissat_add(solver, lit);
	kissat_add(solver, 0);
}

void KissatSolver::add_clause_2(int32_t lit1, int32_t lit2) {
	kissat_add(solver, lit1);
	kissat_add(solver, lit2);
	kissat_add(solver, 0);

}

void KissatSolver::add_clause_3(int32_t lit1, int32_t lit2, int32_t lit3) {
	kissat_add(solver, lit1);
    kissat_add(solver, lit2);
	kissat_add(solver, lit3);
	kissat_add(solver, 0);
}

void KissatSolver::assume(int32_t lit) {
	std::cerr << "Assumptions not supported for this SAT-Solver" << std::endl;
	exit(1);
}

int KissatSolver::solve() {
	int sat = kissat_solve(solver);

	if (sat == SAT_V) {
		model.clear();
		for (int32_t i = 1; i <= decision_vars; i++)
			model.push_back(kissat_value(solver, i) > 0 ? true : false);
	}
	return sat;
}

int KissatSolver::solve(const std::vector<int32_t> assumptions) {
	for (auto const& ass: assumptions) {
		assume(ass);
	}
	return solve();
}
#endif