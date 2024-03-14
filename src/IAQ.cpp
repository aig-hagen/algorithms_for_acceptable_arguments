#include "Algorithms.h"

using namespace std;

namespace Algorithms {
    std::vector<std::string> alg_iaq_cred_co(const AF & af) {
        std::vector<std::string> result;
        ExternalSatSolver solver = ExternalSatSolver(af.count, af.solver_path);
        Encodings.complete(af, solver);

        for (uint32_t i = 0; i < af.args; i++) {
            if (solver.solve({af.accepted_var[i]}) == 10) {
                result.push_back(af.int_to_str[i]);
            }
        }
        return result;
    }

    std::vector<std::string> alg_iaq_cred_st(const AF & af) {
        std::vector<std::string> result;
        ExternalSatSolver solver = ExternalSatSolver(af.count, af.solver_path);
        Encodings.stable(af, solver);

        for (uint32_t i = 0; i < af.args; i++) {
            if (solver.solve({af.accepted_var[i]}) == 10) {
                result.push_back(af.int_to_str[i]);
            }
        }
        return result;
    }
}