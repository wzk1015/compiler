#include "Grammar.h"
#include "MipsGenerator.h"

using namespace std;

int main() {
    cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
	cout << "::                                                 ::" << endl;
	cout << "::              wzk's compiler  V0.1               ::" << endl;
	cout << "::                                                 ::" << endl;
	cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
    Grammar grammar("testfile.txt", "output.txt");
//    Grammar grammar("testfile.txt");
    grammar.analyze();

    Errors::save_to_file("error.txt");

//    if (DEBUG) {
//        SymTable::show();
//    }

    MidCodeList::refactor();
    MidCodeList::save_to_file("midCode.txt");
    MidCodeList::interpret();
    //MidCodeList::show();
    MidCodeList::save_to_file("midCode.txt");

    MipsGenerator mips(MidCodeList::codes, MidCodeList::strcons);
    mips.translate();
    mips.save_to_file("mips.txt");

//    SymTable::show();
    Errors::terminate();
    return 0;
}
