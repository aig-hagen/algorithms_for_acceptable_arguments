#ifndef ENCODINGS_H
#define ENCODINGS_H

#include "AF.h"

#if defined(SAT_CMSAT)
#include "CryptoMiniSatSolver.h"
typedef CryptoMiniSatSolver SAT_Solver;
#elif defined(SAT_CADICAL)
#include "CadicalSatSolver.h"
typedef CadicalSatSolver SAT_Solver;
#elif defined(SAT_GLUCOSE)
#include "GlucoseSatSolver.h"
typedef GlucoseSatSolver SAT_Solver;
#elif defined(SAT_CGSS2)
#include "Cgss2MaxSatSolver.h"
typedef Cgss2MaxSatSolver SAT_Solver;
#elif defined(SAT_EXTERNAL)
#include "ExternalSatSolver.h"
typedef ExternalSatSolver SAT_Solver;
#else
#error "No SAT Solver defined"
#endif

namespace Encodings {
    void admissible(const AF & af, SAT_Solver & solver);
    void complete(const AF & af, SAT_Solver & solver);
    void stable(const AF & af, SAT_Solver & solver);
    void add_rejected_clauses(const AF & af, SAT_Solver & solver);
    void add_conflict_free(const AF & af, SAT_Solver & solver);
    void add_admissible(const AF & af, SAT_Solver & solver);
    void add_complete(const AF & af, SAT_Solver & solver);
}
#endif