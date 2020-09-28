#include "Lexer.h"
#include <stdlib.h>

using namespace std;


int main() {
    system(string(R"(python -c 'print(" ".join([str(i) for i in range(1,101)])," ".join([str(i) for i in range(99,0,-1)]))')").c_str());
//    Lexer lexer = Lexer(true);
//    lexer.analyze("testfile.txt", "output.txt");
    return 0;
}
