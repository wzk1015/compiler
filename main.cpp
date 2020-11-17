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

    if (Errors::terminate()) {
        return 0;
    }

    MidCodeList::save_to_file("midCode_old.txt");
    MidCodeList::refactor();
    //MidCodeList::save_to_file("midCode_old.txt");
    MidCodeList::remove_redundant_assign();
    //MidCodeList::save_to_file("midCode_new.txt");
    //MidCodeList::interpret();
    //MidCodeList::show();
    MidCodeList::save_to_file("midCode.txt");

    MipsGenerator mips(MidCodeList::codes, MidCodeList::strcons);
    mips.translate();
    mips.save_to_file("mips.txt");

    SymTable::show();
    Errors::terminate();

    return 0;
}
