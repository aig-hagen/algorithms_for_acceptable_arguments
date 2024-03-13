/*!
 * The following is largely taken from the mu-toksia solver
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

#include <iostream>
#include "AF.h"

using namespace std;

AF::AF() : args(0), count(0) {}

void AF::add_argument(string arg)
{
	int_to_arg.push_back(arg);
	arg_to_int[arg] = args++;
}

void AF::add_attack(pair<string,string> att)
{
	if (arg_to_int.count(att.first) == 0 || arg_to_int.count(att.second) == 0) {
		return;
	}
	uint32_t source = arg_to_int[att.first];
	uint32_t target = arg_to_int[att.second];
	attackers[target].push_back(source);
	attacked[source].push_back(target);
	unattacked[target] = false;
	if (source == target) {
		self_attack[source] = true;
	}
	att_exists[make_pair(source, target)] = true;
	if (att_exists.count(make_pair(target, source)) > 0) {
		symmetric_attack[make_pair(source, target)] = true;
		symmetric_attack[make_pair(target, source)] = true;
	} else {
		symmetric_attack[make_pair(source, target)] = false;
	}
}

void AF::initialize_attackers() {
	attackers.resize(args);
	attacked.resize(args);
	unattacked.resize(args, true);
	self_attack.resize(args);
}

void AF::initialize_vars() {
	accepted_var.resize(args);
	for (uint32_t i = 0; i < args; i++) {
		accepted_var[i] = ++count;
	}

	rejected_var.resize(args);
	for (uint32_t i = 0; i < args; i++) {
		rejected_var[i] = ++count;
	}
}

void AF::set_solver_path(string path) {
	solver_path = path;
}