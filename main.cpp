#include "Lexer.h"
#include "Grammar.h"
#include "Error.h"

using namespace std;


int main() {
    cout << "wzk's compiler is running..." << endl;
    Grammar grammar("testfile.txt", "output.txt");
    grammar.analyze();

    if (Errors::terminate()) {
        throw exception();
    }

    if (DEBUG) {
        SymTable::show();
    }

    return 0;
}
