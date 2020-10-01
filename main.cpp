#include "Lexer.h"
#include "Grammar.h"
#include "Error.h"

using namespace std;


int main() {
    Lexer lexer(false);
    vector<LexResults> lex_results = lexer.analyze("testfile.txt", "-");

    Grammar grammar(lex_results, true);
    vector<GrammarResults> grammar_results = grammar.analyze("output.txt");

    if (Errors::terminate()) {
        throw exception();
    }

    return 0;
}
