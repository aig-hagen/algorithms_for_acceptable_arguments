#if defined(EEE)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> eee_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> included(af.args+1);
        ExternalSatSolver solver = ExternalSatSolver(af.count, af.solver_path);

        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        std::vector<int> complement_clause;
        complement_clause.reserve(af.args);
        while (true) {
            complement_clause.clear();
            int ret = solver.solve();
            if (ret == 20) break;

            for (uint32_t i = 1; i <= af.args; i++) {
                if (solver.model[i]) {
                    if (!included[i]) {
                        included[i] = true;
                        result.push_back(af.int_to_arg[i-1]);
                    }
                } else {
                    // TODO check correctness
                    complement_clause.push_back(i);
                }
            }
            solver.addClause(complement_clause);
        }

        return result;
    }

    std::vector<std::string> eee_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        std::vector<bool> included(af.args+1, true);
        ExternalSatSolver solver = ExternalSatSolver(af.count, af.solver_path);
        if (sem == PR) {
            // TODO implement maximisation
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        std::vector<int> complement_clause;
        complement_clause.reserve(af.args);
        while (true) {
            int ret = solver.solve();
            if (ret == 20) break;

            for (uint32_t i = 1; i <= af.args; i++) {
                included[i] = included[i] && solver.model[i];
                if (!solver.model[i]) {
                    // TODO check correctness
                    complement_clause.push_back(i);
                }
            }
            solver.addClause(complement_clause);
        }

        for (uint32_t i = 1; i <= af.args; i++) {
            if (included[i]) {
                result.push_back(af.int_to_arg[i-1]);
            }
        }

        return result;
    }
}

#endif