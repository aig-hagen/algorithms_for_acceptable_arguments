#include "Algorithms.h"


namespace Algorithms {
    std::vector<std::string> eee_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        
        SAT_Solver solver = SAT_Solver(af.count, af.args);
        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            //std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }
        std::vector<bool> accepted(af.args, false);
        while (true) {
            int sat = solver.solve();
            if (sat == 20) break;

            std::vector<int32_t> complement_clause;
            complement_clause.reserve(af.count);
            for (uint32_t i = 0; i < af.args; i++) {
                if (solver.model[af.accepted_var[i]]) {
                    complement_clause.push_back(-af.accepted_var[i]);
                } else {
                    complement_clause.push_back(af.accepted_var[i]);
                }
                if (solver.model[af.rejected_var[i]]) {
                    complement_clause.push_back(-af.rejected_var[i]);
                } else {
                    complement_clause.push_back(af.rejected_var[i]);
                }
                if (solver.model[af.accepted_var[i]] && accepted[i] == false) {
                    accepted[i] = true;
                    result.push_back(af.int_to_arg[i]);
                }
            }
            solver.add_clause(complement_clause);
        }

        return result;
    }

    std::vector<std::string> eee_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> included(af.args+1, true);
        SAT_Solver solver = SAT_Solver(af.count, af.args);
        if (sem == PR) {
            Encodings::complete(af, solver);

            std::vector<int32_t> assumptions;
            std::vector<int32_t> complement_clause;
            assumptions.reserve(af.args);
            complement_clause.reserve(af.args);
            while (true) {
                int sat = solver.solve();
                if (sat == 20) break;

                assumptions.clear();
                std::vector<bool> visited(af.args);
                while (true) {
                    complement_clause.clear();
                    for (uint32_t i = 1; i <= af.args; i++) {
                        if (solver.model[i]) {
                            if (!visited[i]) {
                                assumptions.push_back(i);
                                visited[i] = true;
                            }
                        } else {
                            complement_clause.push_back(i);
                        }
                    }
                    solver.add_clause(complement_clause);
                    int superset_exists = solver.solve(assumptions);
                    if (superset_exists == 20) break;
                }
                for (uint32_t i = 1; i <= af.args; i++) {
                    included[i] = included[i] && solver.model[i];
                }
            }

        } else if (sem == ST) {
            Encodings::stable(af, solver);
            std::vector<int32_t> complement_clause;
            complement_clause.reserve(af.count);
            while (true) {
                int sat = solver.solve();
                if (sat == 20) break;

                for (uint32_t i = 1; i <= af.count; i++) {
                    if (i <= af.args) {
                        included[i] = included[i] && solver.model[i];
                    }
                    if (solver.model[i]) {
                        complement_clause.push_back(-i);
                    } else {
                        complement_clause.push_back(i);
                    }
                }
                solver.add_clause(complement_clause);
                complement_clause.clear();
            }
        } else {
            //std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        for (uint32_t i = 1; i <= af.args; i++) {
            if (included[i]) {
                result.push_back(af.int_to_arg[i-1]);
            }
        }

        return result;
    }
}