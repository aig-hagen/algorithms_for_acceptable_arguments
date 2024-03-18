#ifdef SAT_CADICAL

#include "CadicalSatSolver.h"

#include <iostream>


CadicalSatSolver::CadicalSatSolver(int32_t n_vars, int32_t n_args) {
    solver = new CaDiCaL::Solver;
	decision_vars = n_vars;
	//solver.set_no_simplify();
	//solver.set_no_equivalent_lit_replacement();
	//solver.set_no_bva();
	//solver.set_no_bve();
	model = std::vector<bool>(n_vars);
}

void CadicalSatSolver::add_clause(const std::vector<int32_t> & clause) {
	for (int32_t i : clause) {
		solver->add(i);
	}
	solver->add(0);
}

void CadicalSatSolver::assume(int32_t lit) {
	solver->assume(lit);
}

int CadicalSatSolver::solve() {
	int sat = solver->solve();

	if ( !( (sat == 10) || (sat == 20) ) ) {
		std::cerr << "Problem" << std::endl;
		exit(1);
	}

	if (sat == 10) {
		model.clear();
		for (int32_t i = 1; i <= decision_vars; i++)
			model[i-1] = (solver->val(i) > 0 ? true : false);
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