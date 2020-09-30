#include "Lexer.h"
#include "Grammar.h"

using namespace std;


int main() {
    vector<Error> errors;

    Lexer lexer(false);
    vector<LexResults> lex_results = lexer.analyze("testfile.txt", "-");
    errors.insert(errors.end(), lexer.errors.begin(), lexer.errors.end());

    Grammar grammar(lex_results, false);
    vector<GrammarResults> grammar_results = grammar.analyze("output.txt");
    errors.insert(errors.end(), grammar.errors.begin(), grammar.errors.end());

    return 0;
}
