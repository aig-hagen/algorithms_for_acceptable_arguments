#include "Encodings.h"

namespace Encodings {
	void admissible(const AF & af, SAT_Solver & solver, bool isAttackedSet) {
		int32_t offset = isAttackedSet ? 2*af.args: 0;
		for (uint32_t i = 0; i < af.args; i++) {
			solver.add_clause_2(-(offset+af.accepted_var[i]), -(offset+af.rejected_var[i]));
			if (af.unattacked[i]) { // TODO grounded extension
				solver.add_clause_1(offset+af.accepted_var[i]);
				continue;
			} // TODO attacked by grounded/unattacked
			std::vector<int32_t> out_clause(af.attackers[i].size()+1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				solver.add_clause_2(-(offset+af.accepted_var[i]), offset+af.rejected_var[af.attackers[i][j]]);
				out_clause[j] = offset+af.accepted_var[af.attackers[i][j]];
			}
			out_clause[out_clause.size()-1] = -(offset+af.rejected_var[i]);
			solver.add_clause(out_clause);
		}
	}

	void attacks(const AF & af, SAT_Solver & solver) { // TODO maybe integrate directly with admissible encoding
		int32_t offset = 2*af.args;
		std::vector<int32_t> attacks_clause(af.attacks);
		uint32_t attack_idx = 0;
		int32_t attack_var = offset + 2*af.args + 1;
		for (uint32_t i = 0; i < af.args; i++) {
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				attacks_clause[attack_idx++] = attack_var;
				solver.add_clause_2(-attack_var, af.accepted_var[af.attackers[i][j]]);
				solver.add_clause_2(-attack_var, offset+af.accepted_var[i]);
				std::vector<int32_t> clause = { attack_var, -(offset+af.accepted_var[i]), -af.accepted_var[af.attackers[i][j]] };
				solver.add_clause(clause);
				attack_var++;
			}
		}
		solver.add_clause(attacks_clause);
	}

	#ifndef PERF_ENC
	void complete(const AF & af, SAT_Solver & solver) {
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			solver.add_clause_3(af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]);
			solver.add_clause_2(-af.accepted_var[i], -af.rejected_var[i]);
			solver.add_clause_2(-af.accepted_var[i], -af.undecided_var[i]);
			solver.add_clause_2(-af.rejected_var[i], -af.undecided_var[i]);

			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				solver.add_clause_1(af.accepted_var[i]);
			} else {
				std::vector<int32_t> attackers_in_clause(af.attackers[i].size() + 1);
				std::vector<int32_t> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					solver.add_clause_2(-af.accepted_var[i], af.rejected_var[af.attackers[i][j]]);
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
		for (uint32_t i = 0; i < af.args; i++) {
			solver.add_clause_2(-af.accepted_var[i], -af.rejected_var[i]);
			if (af.unattacked[i]) {
				solver.add_clause_1(af.accepted_var[i]);
				continue;
			} // TODO grounded?
			std::vector<int32_t> attackers_in_clause(af.attackers[i].size() + 1);
			std::vector<int32_t> attackers_notout_clause(af.attackers[i].size() + 1);
			for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
				solver.add_clause_2(-af.accepted_var[i], af.rejected_var[af.attackers[i][j]]);
				solver.add_clause_2(-af.accepted_var[af.attackers[i][j]], af.rejected_var[i]);
				attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
				attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
			}
			attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
			solver.add_clause(attackers_in_clause);
			attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
			solver.add_clause(attackers_notout_clause);
		}
	}
	#endif
	#ifdef EXPERIMENTAL
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
				std::vector<int32_t> alt_clause = { af.accepted_var[i], -af.rejected_var[i], -af.undecided_var[i] }; // TODO this seems wrong
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

	void stable(const AF & af, SAT_Solver & solver) { // TODO check correctness
		for (uint32_t i = 0; i < af.args; i++) {
			// basic clauses
			//solver.add_clause_3(af.accepted_var[i], af.rejected_var[i], af.undecided_var[i]);
			solver.add_clause_2(-af.accepted_var[i], -af.rejected_var[i]);
			//solver.add_clause_2(-af.accepted_var[i], -af.undecided_var[i]);
			//solver.add_clause_2(-af.rejected_var[i], -af.undecided_var[i]);
			
			// semantic enconding
			if (af.unattacked[i]) {
				// argument is unattacked
				solver.add_clause_1(af.accepted_var[i]);
			} else {
				//solver.add_clause_1(-af.undecided_var[i]);
				std::vector<int32_t> attackers_in_clause(af.attackers[i].size() + 1);
				std::vector<int32_t> attackers_notout_clause(af.attackers[i].size() + 1);
				for (uint32_t j = 0; j < af.attackers[i].size(); j++) {
					attackers_in_clause[j] = af.accepted_var[af.attackers[i][j]];
					attackers_notout_clause[j] = -af.rejected_var[af.attackers[i][j]];
					solver.add_clause_2(-af.accepted_var[i], af.rejected_var[af.attackers[i][j]]);
				}
				attackers_in_clause[af.attackers[i].size()] = -af.rejected_var[i];
				solver.add_clause(attackers_in_clause);
				attackers_notout_clause[af.attackers[i].size()] = af.accepted_var[i];
				solver.add_clause(attackers_notout_clause);
			}
		}
	}
}