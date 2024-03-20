#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Encodings.h"
#include <iostream>			//std::cout

namespace Algorithms {

#if defined(IAQ)
std::vector<std::string> iaq_cred(const AF & af, semantics sem);
std::vector<std::string> iaq_skep(const AF & af, semantics sem); 
#elif defined(EEE)
// EEE
std::vector<std::string> eee_cred(const AF & af, semantics sem);
std::vector<std::string> eee_skep(const AF & af, semantics sem);
#elif defined(SEE)
// SEE
std::vector<std::string> see_cred(const AF & af, semantics sem);
std::vector<std::string> see_skep(const AF & af, semantics sem);
#elif defined(SEEM)
// SEEM
std::vector<std::string> seem_cred(const AF & af, semantics sem);
std::vector<std::string> seem_skep(const AF & af, semantics sem);
#elif defined(FUDGE)
// fudge
std::vector<std::string> fudge_skep(const AF & af);
#else
#error "undefined"
#endif

// helper methods
bool ds_preferred(const AF & af, int arg);

}

#endif