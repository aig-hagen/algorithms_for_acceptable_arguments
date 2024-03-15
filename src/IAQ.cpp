#if defined(IAQ)

#include "Algorithms.h"

namespace Algorithms {
    std::vector<std::string> iaq_cred(const AF & af, semantics sem) {
        std::vector<std::string> result;
        SAT_Solver solver = SAT_Solver(af.count, af.args);

        if (sem == CO || sem == PR) {
            Encodings::complete(af, solver);
        } else if (sem == ST) {
            Encodings::stable(af, solver);
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }

        for (uint32_t i = 0; i < af.args; i++) {
            solver.assume(af.accepted_var[i]);
            if (solver.solve() == 10) {
                result.push_back(af.int_to_arg[i]);
            }
        }
        return result;
    }

    std::vector<std::string> iaq_skep(const AF & af, semantics sem) {
        std::vector<std::string> result;
        SAT_Solver solver = SAT_Solver(af.count, af.args);
        
        if (sem == PR) {
            for (uint32_t i = 0; i < af.args; i++) {
                if (ds_preferred(af, i)) {
                    result.push_back(af.int_to_arg[i]);
                }
            }
        } else if (sem == ST) {
            Encodings::stable(af, solver);
            for (uint32_t i = 0; i < af.args; i++) {
                solver.assume(-af.accepted_var[i]);
                if (solver.solve() == 20) {
                    result.push_back(af.int_to_arg[i]);
                }
            }
        } else {
            std::cerr << sem << ": Unsupported semantics\n";
            exit(1);
        }
        return result;
    }
}
#endif