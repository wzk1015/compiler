#include "Lexer.h"
#include "Grammar.h"
#include "Error.h"

using namespace std;

int main() {
    cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
	cout << "::                                                 ::" << endl;
	cout << "::              wzk's compiler  V0.1               ::" << endl;
	cout << "::                                                 ::" << endl;
	cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
//    Grammar grammar("testfile.txt", "output.txt");
    Grammar grammar("testfile.txt", "output.txt");
    grammar.analyze();

    Errors::save_to_file("error.txt");

//    if (Errors::terminate()) {
//        throw exception();
//    }

//    if (DEBUG) {
//        SymTable::show();
//    }

    return 0;
}
