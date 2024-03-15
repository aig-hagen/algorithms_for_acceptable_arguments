#include "Algorithms.h"						// Header for all Algorithm methods

//#include <algorithm>						// std::find

namespace Algorithms {

/*mutoksia version of ds-pr*/
bool ds_preferred(const AF & af, int arg) {
	SAT_Solver solver = SAT_Solver(af.count, af.args);
	Encodings::add_complete(af, solver);

	std::vector<int32_t> assumptions = { -af.accepted_var[arg] };

	while (true) {
		int sat = solver.solve(assumptions);
		if (sat == 20) break;

		std::vector<int32_t> complement_clause;
		complement_clause.reserve(af.args);
		std::vector<uint8_t> visited(af.args);
		std::vector<int32_t> new_assumptions = assumptions;
		new_assumptions.reserve(af.args);

		while (true) {
			complement_clause.clear();
			for (int32_t i = 1; i <= af.args; i++) {
				if (solver.model[i]) {
					if (!visited[i]) {
						new_assumptions.push_back(i);
						visited[i] = 1;
					}
				} else {
					complement_clause.push_back(i);
				}
			}
			solver.add_clause(complement_clause);
			int superset_exists = solver.solve(new_assumptions);
			if (superset_exists == 20) break;
		}

		new_assumptions[0] = -new_assumptions[0];

		if (solver.solve(new_assumptions) == 20) {
			return false;
		}
	}
	return true;
}

}