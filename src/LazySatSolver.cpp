#ifdef SAT_CADICAL

#include "LazySatSolver.h"

#include <iostream>


LazySatSolver::LazySatSolver(int32_t n_vars, int32_t n_args) {
    solver = new CaDiCaL::Solver;
	decision_vars = n_args;
	model = std::vector<bool>(decision_vars);
    clauses = std::vector<std::vector<int32_t>>(n_args*n_args);

    num_clauses = 0;
}

void LazySatSolver::add_clause(const std::vector<int32_t> & clause) {
    clauses.push_back(clause);
	num_clauses++;
    clauses[num_clauses-1].push_back(0);
}

void LazySatSolver::add_clause_1(int32_t lit) {
    clauses.push_back({lit, 0});
    num_clauses++;
}

void LazySatSolver::add_clause_2(int32_t lit1, int32_t lit2) {
	clauses.push_back({lit1, lit2, 0});
    num_clauses++;
}

void LazySatSolver::add_clause_3(int32_t lit1, int32_t lit2, int32_t lit3) {
	clauses.push_back({lit1, lit2, lit3, 0});
    num_clauses++;
}

void LazySatSolver::add_temp_clause(const std::vector<int32_t> & clause) {
	for (int32_t i : clause) {
		solver->add(i);
	}
	solver->add(0);
}

void LazySatSolver::add_temp_clause_1(int32_t lit) {
	solver->add(lit);
	solver->add(0);
}

void LazySatSolver::add_temp_clause_2(int32_t lit1, int32_t lit2) {
	solver->add(lit1);
	solver->add(lit2);
	solver->add(0);
}

void LazySatSolver::add_temp_clause_3(int32_t lit1, int32_t lit2, int32_t lit3) {
	solver->add(lit1);
	solver->add(lit2);
	solver->add(lit3);
	solver->add(0);
}

void LazySatSolver::assume(int32_t lit) {
	solver->assume(lit);
}

int LazySatSolver::solve() {
    for (std::vector<int32_t> const & clause : clauses) {
        for (int32_t lit : clause) {
            solver->add(lit);
        }
    }
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
    solver = new CaDiCaL::Solver;
	return sat;
}

int LazySatSolver::solve(const std::vector<int32_t> assumptions) {
	for (auto const& ass: assumptions) {
		assume(ass);
	}
	return solve();
}
#endif