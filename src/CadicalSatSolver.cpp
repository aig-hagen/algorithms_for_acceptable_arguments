#ifdef SAT_CADICAL

#include "CadicalSatSolver.h"

#include <iostream>


CadicalSatSolver::CadicalSatSolver(int32_t n_vars, int32_t n_args) {
    solver = new CaDiCaL::Solver;
	decision_vars = n_vars;
	model = std::vector<bool>(decision_vars);
}

void CadicalSatSolver::add_clause(const std::vector<int32_t> & clause) {
	for (int32_t i : clause) {
		solver->add(i);
	}
	solver->add(0);
}

void CadicalSatSolver::add_clause_1(int32_t lit) {
	solver->add(lit);
	solver->add(0);
}

void CadicalSatSolver::add_clause_2(int32_t lit1, int32_t lit2) {
	solver->add(lit1);
	solver->add(lit2);
	solver->add(0);
}

void CadicalSatSolver::assume(int32_t lit) {
	solver->assume(lit);
}

int CadicalSatSolver::solve() {
	int sat = solver->solve();

	if ( !( (sat == SAT_V) || (sat == UNSAT_V) ) ) {
		std::cerr << "Problem" << std::endl;
		exit(1);
	}

	if (sat == SAT_V) {
		model.clear();
		for (int32_t i = 1; i <= decision_vars; i++)
			model.push_back(solver->val(i) > 0 ? true : false);
	}
	return sat;
}

int CadicalSatSolver::solve(const std::vector<int32_t> assumptions) {
	for (auto const& ass: assumptions) {
		assume(ass);
	}
	return solve();
}
#endif