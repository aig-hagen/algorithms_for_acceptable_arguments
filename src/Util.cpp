#include "Util.h"

//#include <fstream>

using namespace std;

void print_extension_ee(const std::vector<string> & extension)
{
	std::cout << "[";
	for (uint32_t i = 0; i < extension.size(); i++) {
		std::cout << extension[i];
		if (i != extension.size()-1) cout << ",";
	}
	std::cout << "]";
}