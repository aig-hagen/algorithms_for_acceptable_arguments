#include "AF.h"

#if defined(SAT_CMSAT)
#include "CryptoMiniSatSolver.h"
typedef CryptoMiniSatSolver SAT_Solver;
#elif defined(SAT_EXTERNAL)
#include "ExternalSatSolver.h"
typedef ExternalSatSolver SAT_Solver;
#else
#error "No solver defined"
#endif

namespace Encodings {
    void complete(const AF & af, SAT_Solver & solver);
    void stable(const AF & af, SAT_Solver & solver);
    void add_rejected_clauses(const AF & af, SAT_Solver & solver);
    void add_conflict_free(const AF & af, SAT_Solver & solver);
    void add_admissible(const AF & af, SAT_Solver & solver);
    void add_complete(const AF & af, SAT_Solver & solver);
}