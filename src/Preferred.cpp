#include "Algorithms.h"						// Header for all Algorithm methods

//#include <algorithm>						// std::find

namespace Algorithms {
	/*mutoksia version of ds-pr*/
	bool ds_preferred(const AF & af, int arg) {
		SAT_Solver solver = SAT_Solver(af.count, af.args);
		Encodings::complete(af, solver);

		std::vector<int32_t> assumptions = { -af.accepted_var[arg] };

		//std::cout << "Checking argument " << af.int_to_arg[arg] << ":" << std::endl;
		while (true) {
			int sat = solver.solve(assumptions);
			if (sat == 20) {
				//std::cout << "No further model exists" << std::endl;
				break;
			}
			//std::cout << "Found model" << std::endl;

			std::vector<int32_t> complement_clause;
			complement_clause.reserve(af.args);
			std::vector<uint8_t> visited(af.args);
			std::vector<int32_t> new_assumptions = assumptions;
			new_assumptions.reserve(af.args);

			//std::cout << "Maximising..." << std::endl;
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
				//std::cout << "Found Supermodel" << std::endl;
			}
			//std::cout << "Maximal Model reached" << std::endl;

			new_assumptions[0] = -new_assumptions[0];

			if (solver.solve(new_assumptions) == 20) {
				//std::cout << "No Other Model with argument found" << std::endl;
				return false;
			}
			//std::cout << "Model could include argument. Continue..." << std::endl;
		}
		return true;
	}

	std::vector<std::vector<uint32_t>> ee_preferred(const AF & af) {
		std::vector<std::vector<uint32_t>> result;
		SAT_Solver solver = SAT_Solver(af.count, af.args);
		Encodings::add_complete(af, solver);

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
			std::vector<uint32_t> extension;
			for (uint32_t i = 1; i <= af.args; i++) {
				if (solver.model[i]) {
					extension.push_back(i-1);
				}
			}
			result.push_back(extension);
		}
		return result;
	}
}