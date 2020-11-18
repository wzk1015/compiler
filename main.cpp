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
    grammar.analyze();
    Errors::save_to_file("error.txt");

    if (Errors::terminate()) {
        return 0;
    }


    MidCodeList::refactor();
    MidCodeList::remove_redundant_assign();
    MidCodeList::save_to_file("midCode_old.txt");
    MidCodeList::const_broadcast();
    MidCodeList::save_to_file("midCode.txt");

    MipsGenerator mips;
    mips.translate();
    mips.save_to_file("mips.txt");

    SymTable::show();

    return 0;
}
