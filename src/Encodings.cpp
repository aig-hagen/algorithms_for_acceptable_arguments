#include "Encodings.h"

namespace Encodings {
	void admissible(const AF & af, SAT_Solver & solver) {
		std::vector<int32_t> clause(2);
		for (uint32_t i = 0; i < af.args; i++) {
			clause = { -af.accepted_var[i], -af.rejected_var[i] };
			solver.add_clause(clause);
			if (af.unattacked[i]) { // TODO grounded extension
				std::vector<int32_t> unattacked_clause = { af.accepted_var[i] };
				solver.add_clause(unattacked_clause);
				continue;
			} // TODO attacked by grounded
			std::vector<int32_t> out_clause(af.attackers[i].size()+1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				clause = { -af.accepted_var[i], af.rejected_var[af.attackers[i][j]] };
				solver.add_clause(clause);
				out_clause[j] = af.accepted_var[af.attackers[i][j]];
			}
			out_clause[out_clause.size()-1] = -af.rejected_var[i];
			solver.add_clause(out_clause);						
		}
	}

	#ifndef PERF_ENC
	void complete(const AF & af, SAT_Solver & solver) {
		std::vector<int32_t> clause(2);
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
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.add_clause(attackers_in_clause);
				attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.add_clause(attackers_notout_clause);
			}	
		}
	}
	#else
	void complete(const AF & af, SAT_Solver & solver) {
		std::vector<int32_t> clause(2);
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
				std::vector<int32_t> alt_clause = { af.accepted_var[i], -af.rejected_var[i], -af.undecided_var[i] };
				solver.add_clause(alt_clause);
			} else {
				std::vector<int32_t> attackers_in_clause(af.attackers[i].size() + 1);
				std::vector<int32_t> attackers_notout_clause(af.attackers[i].size() + 1);
				std::vector<int32_t> attackers_undecided_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					attackers_undecided_clause[j] = af.undecided_var[af.attackers[i][j]];
					clause = { -af.undecided_var[i], -af.accepted_var[af.attackers[i][j]] };
					solver.add_clause(clause);
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.add_clause(attackers_in_clause);
				attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.add_clause(attackers_notout_clause);
				attackers_undecided_clause[af.attackers[i].size()] = -af.undecided_var[i];
				solver.add_clause(attackers_undecided_clause);
			}	
		}
	}
	#endif

	void stable(const AF & af, SAT_Solver & solver) {
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
}