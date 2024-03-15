#include "Encodings.h"

namespace Encodings {
	void complete(const AF & af, SAT_Solver & solver) {
		std::vector<int> clause(2);
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			clause = { af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]};
			solver.addClause(clause);
			clause = { -af.accepted_var[i], -af.rejected_var[i] };
			solver.addClause(clause);
			clause = { -af.accepted_var[i], -af.undecided_var[i] };
			solver.addClause(clause);
			clause = { -af.rejected_var[i], -af.undecided_var[i] };
			solver.addClause(clause);

			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				std::vector<int> alt_clause = { af.accepted_var[i] };
				solver.addClause(alt_clause);
			} else {
				std::vector<int> attackers_in_clause(af.attackers[i].size() + 1);
				std::vector<int> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
					solver.addClause(clause);
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.addClause(attackers_in_clause);
				attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.addClause(attackers_notout_clause);
			}	
		}
	}

	void stable(const AF & af, SAT_Solver & solver) {
		std::vector<int> clause(2);
		std::vector<int> alt_clause(1);
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			clause = { af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]};
			solver.addClause(clause);
			clause = { -af.accepted_var[i], -af.rejected_var[i] };
			solver.addClause(clause);
			clause = { -af.accepted_var[i], -af.undecided_var[i] };
			solver.addClause(clause);
			clause = { -af.rejected_var[i], -af.undecided_var[i] };
			solver.addClause(clause);
			
			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				alt_clause = { af.accepted_var[i] };
				solver.addClause(alt_clause);
			} else {
				alt_clause = { -af.undecided_var[i] };
				solver.addClause(alt_clause);
				std::vector<int> attackers_in_clause(af.attackers[i].size() + 1);
				std::vector<int> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
					solver.addClause(clause);
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.addClause(attackers_in_clause);
				attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.addClause(attackers_notout_clause);
			}
		}
	}

	#ifdef REL
	void add_rejected_clauses(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			std::vector<int> additional_clause = { -af.rejected_var[i], -af.accepted_var[i] };
			solver.addClause(additional_clause);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				std::vector<int> clause = { af.rejected_var[i], -af.accepted_var[af.attackers[i][j]] };
				solver.addClause(clause);
			}
			std::vector<int> clause(af.attackers[i].size() + 1);
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
				std::vector<int> clause;
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
				if (af.symmetric_attack.at(std::make_pair(af.attackers[i][j], i))) continue;
				std::vector<int> clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
				solver.addClause(clause);
			}
		}
	}

	void add_complete(const AF & af, SAT_Solver & solver) {
		add_admissible(af, solver);
		for (uint32_t i = 0; i < af.args; i++) {
			std::vector<int> clause(af.attackers[i].size()+1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				clause[j] = -af.rejected_var[af.attackers[i][j]];
			}
			clause[af.attackers[i].size()] = af.accepted_var[i];
			solver.addClause(clause);
		}
	}
	#endif

}