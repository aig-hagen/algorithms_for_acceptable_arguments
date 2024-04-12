#if defined(FUDGE)
#include "Algorithms.h"

using namespace std;

namespace Algorithms {
    std::vector<std::string> fudge_skep(const AF & af) {
        std::vector<std::string> result; // T
        std::vector<bool> accepted(af.args); // T
        std::vector<bool> candidate(af.args, true); // C
        int32_t num_candidates = af.args;

        // AdmExtAtt
        SAT_Solver solver2 = SAT_Solver(2*af.args, af.args);
        #ifdef SAT_EXTERNAL
        solver2.set_solver(af.solver_path);
        #endif

        // AdmExt
        SAT_Solver solver3 = SAT_Solver(2*af.args, af.args);
        Encodings::admissible(af, solver3);
        #ifdef SAT_EXTERNAL
        solver3.set_solver(af.solver_path);
        #endif
        
        while (true) {
            // C = \emptyset ?
            if (num_candidates == 0) return result;

            // E := AdmExt'(af, T, C)
            SAT_Solver solver1 = SAT_Solver(2*af.args, af.args);
            #ifdef SAT_EXTERNAL
            solver1.set_solver(af.solver_path);
            #endif
            Encodings::admissible(af, solver1);
            std::vector<int32_t> intersect_clause;
            intersect_clause.reserve(af.args);
            for (uint32_t i = 0; i < af.args; i++) {
                if (candidate[i]) {
                    intersect_clause.push_back(af.accepted_var[i]);
                }
                if (accepted[i]) {
                    solver1.assume(af.accepted_var[i]);
                }
            }
            solver1.add_clause(intersect_clause);
            if (solver1.solve() == UNSAT_V) {
                return result;
            }

            // C := C \setminus E+
            for (uint32_t i = 0; i < af.args; i++) {
                if (!solver1.model[i]) continue;
                for (uint32_t j = 0; j < af.attacked[i].size(); j++) {
                    if (candidate[af.attacked[i][j]]) {
                        candidate[af.attacked[i][j]] = false;
                        num_candidates--;
                    }
                }
            }

            for (uint32_t a = 0; a < af.args; a++) {
                if (!solver1.model[a] || candidate[a]) continue;
                std::vector<uint32_t> set; // S TODO needed?
                set.reserve(af.args);
                std::vector<bool> included(af.args); // S

                while (true) {
                    // E' := AdmExtAtt(af, T \cup {a}, S)
                    int hasExtensionAttacking = NULL; // TODO add call AdmExtAtt
                    if (hasExtensionAttacking == UNSAT_V) {
                        // T := T \cup {a}
                        accepted[a] = true;
                        result.push_back(af.int_to_arg[a]);
                        // C := C \setminus {a}
                        candidate[a] = false;
                        num_candidates--;
                        break;
                    }

                    // C := C \setminus E'+
                    for (uint32_t i = 0; i < af.args; i++) {
                        if (!solver2.model[i]) continue;
                        for (uint32_t j = 0; j < af.attacked[i].size(); j++) {
                            if (candidate[af.attacked[i][j]]) {
                                candidate[af.attacked[i][j]] = false;
                                num_candidates--;
                            }
                        }
                    }

                    // E'' = AdmExt(af, E' \cup T \cup {a})
                    for (uint32_t i = 0; i < af.args; i++) {
                        if (a==i || solver2.model[i] || accepted[i]) {
                            solver3.assume(af.accepted_var[i]);
                        }
                    }
                    if (solver3.solve() == UNSAT_V) {
                        // C := C \setminus {a}
                        candidate[a] = false;
                        num_candidates--;
                        break;
                    }

                    // C := C \setminus E''+
                    for (uint32_t i = 0; i < af.args; i++) {
                        if (!solver3.model[i]) continue;
                        for (uint32_t j = 0; j < af.attacked[i].size(); j++) {
                            if (candidate[af.attacked[i][j]]) {
                                candidate[af.attacked[i][j]] = false;
                                num_candidates--;
                            }
                        }
                    }

                    // S := S \cup E''
                    for (uint32_t i = 0; i < af.args; i++) {
                        if (solver3.model[i] && !included[i]) {
                            included[i] = true;
                            set.push_back(i);
                        }
                    }
                }
            }
        }
    }
}

#endif