#include "Encodings.h"
//#include <algorithm>

using namespace std;

namespace Encodings {
	void complete(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			solver.addClause({ af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]});
			solver.addClause({ -af.accepted_var[i], -af.rejected_var[i] });
			solver.addClause({ -af.accepted_var[i], -af.undecided_var[i] });
			solver.addClause({ -af.rejected_var[i], -af.undecided_var[i] });

			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				solver.addClause({ af.accepted_var[i] });
			} else {
				vector<int> attackers_in_clause(af.attackers[i].size() + 1);
				vector<int> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]]
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					solver.addClause({ -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] });
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.addClause(attackers_in_clause);
				attacker_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.addClause(attackers_notout_clause);
			}	
		}
	}

	void stable(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			solver.addClause({ af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]});
			solver.addClause({ -af.accepted_var[i], -af.rejected_var[i] });
			solver.addClause({ -af.accepted_var[i], -af.undecided_var[i] });
			solver.addClause({ -af.rejected_var[i], -af.undecided_var[i] });
			
			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				solver.addClause({ af.accepted_var[i] });
			} else {
				solver.addClause({ -af.undecided_var[i] });
				vector<int> attackers_in_clause(af.attackers[i].size() + 1);
				vector<int> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]]
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					solver.addClause({ -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] });
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.addClause(attackers_in_clause);
				attacker_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.addClause(attackers_notout_clause);
			}
		}
	}

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