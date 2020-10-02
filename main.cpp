#include "Lexer.h"
#include "Grammar.h"
#include "Error.h"

using namespace std;


int main() {
    Grammar grammar("testfile.txt", "output.txt");
    vector<GrammarResults> grammar_results = grammar.analyze();

    if (Errors::terminate()) {
        throw exception();
    }

    return 0;
}
