#include "lang.hpp"
int main() {
    try {
        std::ifstream f("cache/code.txt");
        Vm v;
        readlns(&v, f);
        v.vmrun(0);
    }
    catch (std::string s){
        std::cout << "error: " << s << std::endl;
    }
}