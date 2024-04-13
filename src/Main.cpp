/*!
 * The following is adapted from the mu-toksia argumentation-solver
 * and is subject to the following licence.
 *
 * 
 * Copyright (c) <2020> <Andreas Niskanen, University of Helsinki>
 * 
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * 
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Algorithms.h"		// Algorithms for all supported Problems
#include <getopt.h>			// parsing commandline options

#include <iostream>			//std::cout
#include <fstream>			//ifstream
#include <sstream>			//istringstream
#include <algorithm>
#include <stack>

static int version_flag = 0;
static int usage_flag = 0;
static int formats_flag = 0;
static int problems_flag = 0;

void print_extension_ee(const std::vector<std::string> & extension) {
	std::cout << "[";
	for (uint32_t i = 0; i < extension.size(); i++) {
		std::cout << extension[i];
		if (i != extension.size()-1) std::cout << ",";
	}
	std::cout << "]";
}

task string_to_task(std::string problem) {
	std::string tmp = problem.substr(0, problem.find("-"));
	if (tmp == "EC") return EC;
	if (tmp == "ES") return ES;
	return UNKNOWN_TASK;
}

semantics string_to_sem(std::string problem) {
	problem.erase(0, problem.find("-") + 1);
	std::string tmp = problem.substr(0, problem.find("-"));
	if (tmp == "CO") return CO;
	if (tmp == "ST") return ST;
	if (tmp == "PR") return PR;
	return UNKNOWN_SEM;
}

void print_usage(std::string solver_name) {
	std::cout << "Usage: " << solver_name << " -p <task> -f <file> -fo <format> \n\n";
	std::cout << "  <task>      computational problem; for a list of available problems use option --problems\n";
	std::cout << "  <file>      input argumentation framework\n";
	std::cout << "  <format>    file format for input AF; for a list of available formats use option --formats\n";
	std::cout << "Options:\n";
	std::cout << "  --help      Displays this help message.\n";
	std::cout << "  --version   Prints version and author information.\n";
	std::cout << "  --formats   Prints available file formats.\n";
	std::cout << "  --problems  Prints available computational tasks.\n";
}

void print_version(std::string solver_name) {
	std::cout << solver_name << " (version 1.0)\n" << "TODO\n";
}

void print_formats() {
	std::cout << "[tgf,i23]\n";
}

void print_problems() {
	#if defined(IAQ) || defined(EEE)
	std::cout << "[EC-CO,EC-PR,EC-ST,ES-ST,ES-PR]\n";
	#elif defined(SEE) || defined(SEEM)
	std::cout << "[EC-CO,EC-PR,EC-ST,ES-ST]\n";
	#elif defined(FUDGE)
	std::cout << "[ES-PR]\n";
	#else
	#endif
}

int main(int argc, char ** argv) {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(NULL);

	if (argc == 1) {
		print_version(argv[0]);
		return 0;
	}

	const struct option longopts[] = {
		{"help", no_argument, &usage_flag, 1},
		{"version", no_argument, &version_flag, 1},
		{"formats", no_argument, &formats_flag, 1},
		{"problems", no_argument, &problems_flag, 1},
		{"p", required_argument, 0, 'p'},
		{"f", required_argument, 0, 'f'},
		{"fo", required_argument, 0, 'o'},
		{"s", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	int option_index = 0;
	int opt = 0;
	std::string task, file, fileformat, query, sat_path;

	while ((opt = getopt_long_only(argc, argv, "", longopts, &option_index)) != -1) {
		switch (opt) {
			case 0:
				break;
			case 'p':
				task = optarg;
				break;
			case 'f':
				file = optarg;
				break;
			case 'o':
				fileformat = optarg;
				break;
			case 's':
				sat_path = optarg;
				break;
			default:
				return 1;
		}
	}

	if (version_flag) {
		print_version(argv[0]);
		return 0;
	}

	if (usage_flag) {
		print_usage(argv[0]);
		return 0;
	}

	if (formats_flag) {
		print_formats();
		return 0;
	}

	if (problems_flag) {
		print_problems();
		return 0;
	}

	if (task.empty()) {
		std::cerr << argv[0] << ": Task must be specified via -p flag\n";
		return 1;
	}

	if (file.empty()) {
		std::cerr << argv[0] << ": Input file must be specified via -f flag\n";
		return 1;
	}

	if (fileformat.empty()) {
		//std::cerr << argv[0] << ": File format must be specified via -fo flag\n";
		fileformat = "i23";
		//return 1;
	}

	std::ifstream input;
	input.open(file);

	if (!input.good()) {
		std::cerr << argv[0] << ": Cannot open input file\n";
		return 1;
	}

	AF aaf = AF();
	aaf.sem = string_to_sem(task);
	int32_t n_args = 0;
	std::string line, arg, source, target;
	if (fileformat == "i23") {
		while (!input.eof()) {
			getline(input, line);
			if (line.length() == 0 || line[0] == '#') continue;
			std::istringstream iss(line);
			if (line[0] == 'p') {
				std::string p, af;
				iss >> p >> af >> n_args;
				for (int i = 1; i <= n_args; i++) {
					aaf.add_argument(std::to_string(i));
				}
				aaf.initialize_attackers();
			} else {
				iss >> source >> target;
				aaf.add_attack(make_pair(source, target));
			}
		}
	} else if (fileformat == "tgf") {
		while (input >> arg) {
			if (arg == "#") {
				aaf.initialize_attackers();
				break;
			}
			aaf.add_argument(arg);
		}
		while (input >> source >> target) {
			aaf.add_attack(make_pair(source, target));
		}
	} else {
		std::cerr << argv[0] << ": Unsupported file format\n";
		return 1;
	}
	input.close();

	aaf.initialize_vars();
	aaf.set_solver_path(sat_path);

	std::vector<std::string> acceptable_arguments;
	switch (string_to_task(task)) {
		case EC:
			#if defined(IAQ)
			//std::cout << "Using IAQ..." << std::endl;
			acceptable_arguments = Algorithms::iaq_cred(aaf, string_to_sem(task));
			#elif defined(EEE)
			//std::cout << "Using EEE..." << std::endl;
			acceptable_arguments = Algorithms::eee_cred(aaf, string_to_sem(task));
			#elif defined(SEE)
			//std::cout << "Using SEE..." << std::endl;
			acceptable_arguments = Algorithms::see_cred(aaf, string_to_sem(task));
			#elif defined(SEEM)
			//std::cout << "Using SEEM..." << std::endl;
			acceptable_arguments = Algorithms::seem_cred(aaf, string_to_sem(task));
			#else
			#endif
			break;
		case ES:
			#if defined(IAQ)
			acceptable_arguments = Algorithms::iaq_skep(aaf, string_to_sem(task));
			#elif defined(EEE)
			acceptable_arguments = Algorithms::eee_skep(aaf, string_to_sem(task));
			#elif defined(SEE)
			acceptable_arguments = Algorithms::see_skep(aaf, string_to_sem(task));
			#elif defined(SEEM)
			acceptable_arguments = Algorithms::seem_skep(aaf, string_to_sem(task));
			#elif defined(FUDGE)
			if (string_to_sem(task)!=PR) {
				std::cerr << string_to_sem(task) << ": Unsupported semantics\n";
            	exit(1);
			}
			acceptable_arguments = Algorithms::fudge_skep(aaf);
			#else
			#endif
			break;
		default:
			std::cerr << argv[0] << ": Problem not supported!\n";
			return 1;
	}
	print_extension_ee(acceptable_arguments);
	std::cout << "\n";

	return 0;
}