#ifndef DIMACS_H
#define DIMACS_H

#include "global.h"

#include<cstdint>
#include <vector>
#include <string>

namespace cgss2{

class DIMACS{
public:
    enum TYPE{SAT, WPMS, WPMS22};
    TYPE type;
    std::string filename;
    std::string error; // read or read_file returns nonzero, this will contain description of error
    uint64_t topv;
    int vars;

    std::vector<std::vector<int> > clauses;
    std::vector<uint64_t> weights;

private:
    void read_clause(std::istream& in, uint64_t w = HARDWEIGHT);
public:
    int read(std::istream& in); // returns 0 if success
    int read_file(std::string fn); // returns 0 if success

    DIMACS(): type(WPMS22), topv(HARDWEIGHT), vars(0) {}
};

}

#endif
