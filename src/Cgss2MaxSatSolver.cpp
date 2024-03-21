#ifdef SAT_CGSS2

#include "Cgss2MaxSatSolver.h"
#include <iostream>


Cgss2MaxSatSolver::Cgss2MaxSatSolver(int32_t n_vars, int32_t n_args) {
    solver = new CGSS2();
    solver->init_default();
    //solver->make_incremental();
	decision_vars = n_vars;
	model = std::vector<bool>(decision_vars);
}

void Cgss2MaxSatSolver::add_clause(const std::vector<int32_t> & clause) {
    for (const uint32_t l: clause) {
        solver->ipamir_add_hard(l);
    }
    solver->ipamir_add_hard(0);
}

void Cgss2MaxSatSolver::add_soft_constraint(int32_t lit) {
	solver->ipamir_add_soft_lit(lit, 1);
}

void Cgss2MaxSatSolver::disable_soft_constraint(int32_t lit) {
	solver->ipamir_add_soft_lit(lit, 0);
}

void Cgss2MaxSatSolver::assume(int32_t lit) {
	solver->ipamir_assume(lit);
}

int Cgss2MaxSatSolver::solve() {
	int sat = solver->ipamir_solve();

    if (sat == ERROR_V) {
        std::cerr << "Problem" << std::endl;
        exit(1);
    }

	if (sat == SAT_V) {
        for (int32_t i = 1; i <= decision_vars; i++) {
		    model[i-1] = solver->ipamir_val_lit(i)>0 ? true: false;
        }
	}
	return sat;
}

int Cgss2MaxSatSolver::solve(const std::vector<int32_t> assumptions) {
	for (auto const& ass: assumptions) {
		assume(ass);
	}
	return solve();
}

uint64_t Cgss2MaxSatSolver::get_optimal_cost() {
    return solver->ipamir_val_obj();
}
#endif