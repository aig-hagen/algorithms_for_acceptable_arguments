#include "Encodings.h"

#include <iostream>

namespace Encodings {
	void complete(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			std::vector<int32_t> clause = { -af.accepted_var[i], -af.rejected_var[i]};
			solver.add_clause(clause);

			std::vector<int32_t> attackersInClause(af.attackers[i].size()+1);
			std::vector<int32_t> attackersNotOutClause(af.attackers[i].size()+1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				std::vector<int32_t> argumentOutClause = { af.rejected_var[i], -af.accepted_var[af.attackers[i][j]] };
				std::vector<int32_t> argumentNotInClause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
				solver.add_clause(argumentOutClause);
				solver.add_clause(argumentNotInClause);
				attackersInClause[j] = af.accepted_var[af.attackers[i][j]];
				attackersNotOutClause[j] = -af.rejected_var[af.attackers[i][j]];
			}
			attackersInClause[af.attackers[i].size()] = -af.rejected_var[i];
			attackersNotOutClause[af.attackers[i].size()] = af.accepted_var[i];
			solver.add_clause(attackersInClause);
			solver.add_clause(attackersNotOutClause);
		}
	}

	void stable(const AF & af, SAT_Solver & solver) {
		
	}

	#if defined(TEST)
	void complete_tweety(const AF & af, SAT_Solver & solver) {
		std::vector<int32_t> clause(2);
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			std::vector<int32_t> clause_a = { af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]};
			solver.add_clause(clause_a);
			clause = { -af.accepted_var[i], -af.rejected_var[i] };
			solver.add_clause(clause);
			clause = { -af.accepted_var[i], -af.undecided_var[i] };
			solver.add_clause(clause);
			clause = { -af.rejected_var[i], -af.undecided_var[i] };
			solver.add_clause(clause);

			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				std::vector<int32_t> alt_clause = { af.accepted_var[i] };
				solver.add_clause(alt_clause);
			} else {
				std::vector<int32_t> attackers_in_clause(af.attackers[i].size() + 1);
				std::vector<int32_t> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
					solver.add_clause(clause);
					clause = { -af.accepted_var[af.attackers[i][j]], af.rejected_var[i] };
					solver.add_clause(clause);
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.add_clause(attackers_in_clause);
				attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.add_clause(attackers_notout_clause);
			}	
		}
	}

	void stable_tweety(const AF & af, SAT_Solver & solver) {
		std::vector<int32_t> clause(2);
		std::vector<int32_t> alt_clause(1);
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			clause = { af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]};
			solver.add_clause(clause);
			clause = { -af.accepted_var[i], -af.rejected_var[i] };
			solver.add_clause(clause);
			clause = { -af.accepted_var[i], -af.undecided_var[i] };
			solver.add_clause(clause);
			clause = { -af.rejected_var[i], -af.undecided_var[i] };
			solver.add_clause(clause);
			
			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				alt_clause = { af.accepted_var[i] };
				solver.add_clause(alt_clause);
			} else {
				alt_clause = { -af.undecided_var[i] };
				solver.add_clause(alt_clause);
				std::vector<int32_t> attackers_in_clause(af.attackers[i].size() + 1);
				std::vector<int32_t> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
					solver.add_clause(clause);
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.add_clause(attackers_in_clause);
				attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.add_clause(attackers_notout_clause);
			}
		}
	}

	void add_rejected_clauses(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			std::vector<int32_t> additional_clause = { -af.rejected_var[i], -af.accepted_var[i] };
			solver.add_clause(additional_clause);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				std::vector<int32_t> clause = { af.rejected_var[i], -af.accepted_var[af.attackers[i][j]] };
				solver.add_clause(clause);
			}
			std::vector<int32_t> clause(af.attackers[i].size() + 1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				clause[j] = af.accepted_var[af.attackers[i][j]];
			}
			clause[af.attackers[i].size()] = -af.rejected_var[i];
			solver.add_clause(clause);
		}
	}

	void add_conflict_free(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				std::vector<int32_t> clause;
				if (i != af.attackers[i][j]) {
					clause = { -af.accepted_var[i], -af.accepted_var[af.attackers[i][j]] };
				} else {
					clause = { -af.accepted_var[i] };
				}
				solver.add_clause(clause);
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
				std::vector<int32_t> clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
				solver.add_clause(clause);
			}
		}
	}

	// somehow doesnt work for skeptical preferred reasoning
	void add_complete(const AF & af, SAT_Solver & solver) {
		add_admissible(af, solver);
		for (uint32_t i = 0; i < af.args; i++) {
			std::vector<int32_t> clause(af.attackers[i].size()+1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				clause[j] = -af.rejected_var[af.attackers[i][j]];
			}
			clause[af.attackers[i].size()] = af.accepted_var[i];
			solver.add_clause(clause);
		}
	}
	#endif
}