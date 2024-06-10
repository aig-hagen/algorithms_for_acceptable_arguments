#include "Algorithms.h"
#include "LazySatSolver.h"

using namespace std;

namespace Algorithms {
    std::vector<std::string> fudge_skep(const AF & af) {
        std::vector<uint8_t> C(af.args, 1);
        uint32_t sizeC = af.args;
        std::vector<uint8_t> T(af.args, 0);

        CadicalSatSolver solver1 = CadicalSatSolver(2*af.args, af.args);
        Encodings::admissible(af, solver1, false);

        while (true) {
            if (sizeC == 0) {
                break;
            }
            
            // E := AdmExt'(AF,T,C)
            // TODO add clause
            if (solver1.solve() == UNSAT_V) {
                break;
            }

            // TODO remove attacked by E

            for (uint32_t a = 0; a < af.args; a++) {
                if (!C[a] || !solver1.model[a]) {
                    continue;
                }
                std::vector<uint8_t> S(af.args, 0);
                
                while (true) {
                    // E' := AdmExtAtt(AF, T \cup {a}, S)
                    
                }
            }
        }
        
    }
}