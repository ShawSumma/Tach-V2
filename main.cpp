// standard header for all TachV2 code
#include "lang.hpp"
// should implement argc and argv
int main() {
    try {
        // errors as std::string, other types may be added
        // if there is an error or no code is ran, create a cache directory
        std::ifstream f("cache/code.txt");
        Vm vm;
        // before i write a new parser (maybe) in C++ you must run one of the python parsers
        readlns(&vm, f);
        opt_required(&vm);
        opt_loadargs(&vm);
        opt_floval(&vm);
        std::ofstream outfile("cache/state.txt");
        vm.savestate(outfile);
        outfile.close();
        //start at opcode 0
        vm.vmrun(0);
    }
    // all errors are std::string for the moment, error handling is not yet to an okay state
    catch (std::string s){
        // as i said, not ready... not even a line number or call trace
        std::cout << "error: " << s << std::endl;
    }
}