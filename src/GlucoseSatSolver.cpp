/*!
 * The following code has been adapted from the mu-toksia project 
 * which is subject of the following license 
 *
 * Copyright (c) <2023> <Andreas Niskanen, University of Helsinki>
 * 
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * 
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef SAT_GLUCOSE

#include "GlucoseSatSolver.h"

using namespace Glucose;

GlucoseSatSolver::GlucoseSatSolver(int32_t n_vars, int32_t n_args) {
	solver = new Solver();
	solver->setIncrementalMode();
	solver->initNbInitialVars(n_args);
	decision_vars = n_args;
}

void GlucoseSatSolver::add_clause(const std::vector<int32_t> & clause) {
	vec<Lit> lits(clause.size());
	for (uint32_t i = 0; i < clause.size(); i++) {
		int32_t var = abs(clause[i])-1;
		while (var >= solver->nVars())
			solver->newVar();
		lits[i] = (clause[i] > 0) ? mkLit(var) : ~mkLit(var);
	}
	solver->addClause_(lits);
}

void GlucoseSatSolver::add_clause_1(int32_t lit) {
	int32_t var = abs(lit)-1;
	while (var >= solver->nVars())
			solver->newVar();
	solver->addClause((lit > 0) ? mkLit(var) : ~mkLit(var));
}

void GlucoseSatSolver::add_clause_2(int32_t lit1, int32_t lit2) {
	int32_t var1 = abs(lit1)-1;
	int32_t var2 = abs(lit2)-1;
	while (var2 >= solver->nVars())
			solver->newVar();
	solver->addClause((lit1 > 0) ? mkLit(var1) : ~mkLit(var1), (lit2 > 0) ? mkLit(var2) : ~mkLit(var2));
}

void GlucoseSatSolver::assume(int32_t lit) {
	int32_t var = abs(lit)-1;
	while (var >= solver->nVars())
		solver->newVar();
	assumptions.push((lit > 0) ? mkLit(var) : ~mkLit(var));
}

int GlucoseSatSolver::solve() {
	int sat = solver->solve(assumptions) ? SAT_V : UNSAT_V;
	if (sat == SAT_V) {
		model.clear();
		for (int32_t i = 0; i < decision_vars; i++) {
			model.push_back(solver->modelValue(i) == l_True ? 1 : 0);
		}
	}
	assumptions.clear();
	return sat;
}

int GlucoseSatSolver::solve(const std::vector<int32_t> assumptions) {
	for (auto const& ass: assumptions) {
		assume(ass);
	}
	return solve();
}

#endif