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

#include "AF.h"				// Modelling of argumentation frameworks
#include "Algorithms.h"		// Algorithms for all supported Problems

#include <iostream>			//std::cout
#include <fstream>			//ifstream
#include <sstream>			//istringstream
#include <algorithm>
#include <stack>

#include <getopt.h>			// parsing commandline options

using namespace std;

static int version_flag = 0;
static int usage_flag = 0;
static int formats_flag = 0;
static int problems_flag = 0;

task string_to_task(string problem) {
	string tmp = problem.substr(0, problem.find("-"));
	if (tmp == "EC") return EC;
	if (tmp == "ES") return ES;
	return UNKNOWN_TASK;
}

semantics string_to_sem(string problem) {
	problem.erase(0, problem.find("-") + 1);
	string tmp = problem.substr(0, problem.find("-"));
	if (tmp == "CO") return CO;
	if (tmp == "ST") return ST;
	if (tmp == "PR") return PR;
	return UNKNOWN_SEM;
}

void print_usage(string solver_name) {
	cout << "Usage: " << solver_name << " -p <task> -f <file> -fo <format> \n\n";
	cout << "  <task>      computational problem; for a list of available problems use option --problems\n";
	cout << "  <file>      input argumentation framework\n";
	cout << "  <format>    file format for input AF; for a list of available formats use option --formats\n";
	cout << "Options:\n";
	cout << "  --help      Displays this help message.\n";
	cout << "  --version   Prints version and author information.\n";
	cout << "  --formats   Prints available file formats.\n";
	cout << "  --problems  Prints available computational tasks.\n";
}

void print_version(string solver_name) {
	cout << solver_name << " (version 1.0)\n" << "TODO\n";
}

void print_formats() {
	cout << "[tgf,i23]\n";
}

void print_problems() {
	cout << "[EC-CO,EC-ST,ES-ST,ES-PR]\n";
}

int main(int argc, char ** argv) {
	ios_base::sync_with_stdio(false);
	cin.tie(NULL);

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
	string task, file, fileformat, query, sat_path;

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
		cerr << argv[0] << ": Task must be specified via -p flag\n";
		return 1;
	}

	if (file.empty()) {
		cerr << argv[0] << ": Input file must be specified via -f flag\n";
		return 1;
	}

	if (fileformat.empty()) {
		cerr << argv[0] << ": File format must be specified via -fo flag\n";
		return 1;
	}

	ifstream input;
	input.open(file);

	if (!input.good()) {
		cerr << argv[0] << ": Cannot open input file\n";
		return 1;
	}

	AF aaf = AF();
	aaf.sem = string_to_sem(task);
	int32_t n_args = 0;
	string line, arg, source, target;
	if (fileformat == "i23") {
		while (!input.eof()) {
			getline(input, line);
			if (line.length() == 0 || line[0] == '#') continue;
			std::istringstream iss(line);
			if (line[0] == 'p') {
				string p, af;
				iss >> p >> af >> n_args;
				for (int32_t i = 1; i <= n_args; i++) {
					aaf.add_argument(i);
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
		cerr << argv[0] << ": Unsupported file format\n";
		return 1;
	}
	input.close();

	aaf.initialize_vars();
	aaf.set_solver_path(sat_path);

	vector<string> acceptable_arguments;
	switch (string_to_task(task)) {
		case EC:
		{
			switch (string_to_sem(task)) {
				case CO:
					acceptable_arguments = Algorithms::alg_iaq_cred_co(aaf);
					break;
				case ST:
					acceptable_arguments = Algorithms::alg_iaq_cred_st(aaf);
					break;
				default:
					cerr << argv[0] << ": Unsupported semantics\n";
					return 1;
			}
			break;
		}
		case ES:
		{
			switch (string_to_sem(task)) {
				case ST:
					acceptable_arguments = Algorithms::alg_iaq(aaf);
					break;
				case PR:
					acceptable_arguments = Algorithms::alg_iaq(aaf);
					break;
				default:
					cerr << argv[0] << ": Unsupported semantics\n";
					return 1;
			}
			break;
		}
		default:
			cerr << argv[0] << ": Problem not supported!\n";
			return 1;
	}
	print_extension_ee(acceptable_arguments);
	cout << "\n";

	return 0;
}