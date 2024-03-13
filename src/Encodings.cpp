#include "Encodings.h"
//#include <algorithm>

using namespace std;

namespace Encodings {
/*!
 * The following is largely taken from the mu-toksia solver
 * and is subject to the following licence.
 *
 * 
 * Copyright (c) <2020> <Andreas Niskanen, University of Helsinki>
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
	void add_rejected_clauses(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			vector<int> additional_clause = { -af.rejected_var[i], -af.accepted_var[i] };
			solver.addClause(additional_clause);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				vector<int> clause = { af.rejected_var[i], -af.accepted_var[af.attackers[i][j]] };
				solver.addClause(clause);
			}
			vector<int> clause(af.attackers[i].size() + 1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				clause[j] = af.accepted_var[af.attackers[i][j]];
			}
			clause[af.attackers[i].size()] = -af.rejected_var[i];
			solver.addClause(clause);
		}
	}

	void add_conflict_free(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				vector<int> clause;
				if (i != af.attackers[i][j]) {
					clause = { -af.accepted_var[i], -af.accepted_var[af.attackers[i][j]] };
				} else {
					clause = { -af.accepted_var[i] };
				}
				solver.addClause(clause);
			}
		}
	}

	void add_admissible(const AF & af, SAT_Solver & solver) {
		add_conflict_free(af, solver);
		add_rejected_clauses(af, solver);
		for (uint32_t i = 0; i < af.args; i++) {
			if (af.self_attack[i]) continue;
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				if (af.symmetric_attack.at(make_pair(af.attackers[i][j], i))) continue;
				vector<int> clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
				solver.addClause(clause);
			}
		}
	}

	void add_complete(const AF & af, SAT_Solver & solver) {
		add_admissible(af, solver);
		for (uint32_t i = 0; i < af.args; i++) {
			vector<int> clause(af.attackers[i].size()+1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				clause[j] = -af.rejected_var[af.attackers[i][j]];
			}
			clause[af.attackers[i].size()] = af.accepted_var[i];
			solver.addClause(clause);
		}
	}

}