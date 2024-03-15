#if defined(IAQ)
#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> iaq_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        ExternalSatSolver solver = ExternalSatSolver(af.count, af.solver_path);

        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        for (uint32_t i = 0; i < af.args; i++) {
            std::vector assumption = {af.accepted_var[i]};
            if (solver.solve(assumption) == 10) {
                result.push_back(af.int_to_arg[i]);
            }
        }
        return result;
    }

    std::vector<std::string> iaq_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        ExternalSatSolver solver = ExternalSatSolver(af.count, af.solver_path);
        
        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        for (uint32_t i = 0; i < af.args; i++) {
            std::vector assumption = {-af.accepted_var[i]};
            if (solver.solve(assumption) == 20) {
                result.push_back(af.int_to_arg[i]);
            }
        }
        return result;
    }
}

#endif