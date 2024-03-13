#include "AF.h"

#if defined(SAT_CMSAT)
#include "CryptoMiniSatSolver.h"
typedef CryptoMiniSatSolver SAT_Solver;
#else
#include "ExternalSatSolver.h"
typedef ExternalSatSolver SAT_Solver;
#endif

namespace Encodings {
    void add_rejected_clauses(const AF & af, SAT_Solver & solver);
    void add_conflict_free(const AF & af, SAT_Solver & solver);
    void add_admissible(const AF & af, SAT_Solver & solver);
    void add_complete(const AF & af, SAT_Solver & solver);
}