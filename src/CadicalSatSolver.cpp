#ifdef SAT_CADICAL

#include "CadicalSatSolver.h"


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
	std::vector<Lit> lits(clause.size());
	for (uint32_t i = 0; i < clause.size(); i++) {
		int32_t var = abs(clause[i])-1;
		while ((uint32_t)var >= solver.nVars())
			solver.new_var();
		lits[i] = Lit(var, clause[i] < 0);
	}
	solver.add_clause(lits);
}

void CadicalSatSolver::assume(int32_t lit) {
	int32_t var = abs(lit)-1;
	while ((uint32_t)var >= solver.nVars())
		solver.new_var();
	assumptions.push_back(Lit(var, lit < 0));
}

int CadicalSatSolver::solve() {
	int sat = solver.solve(&assumptions) == l_True ? 10 : 20;
	if (sat == 10) {
		model.clear();
		for (int32_t i = 0; i < decision_vars; i++)
			model[i] = (solver.get_model()[i] == l_True ? 1 : 0);
	}
	assumptions.clear();
	return sat;
}

int CadicalSatSolver::solve(const std::vector<int32_t> assumptions) {
	for (auto const& ass: assumptions) {
		assume(ass);
	}
	return solve();
}

bool CadicalSatSolver::propagate(std::vector<int32_t> & out_lits) {
	return false;
}

bool CadicalSatSolver::get_value(int32_t lit) {
    return false;
}
#endif