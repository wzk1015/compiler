#include "Grammar.h"
#include "MipsGenerator.h"

using namespace std;

int main() {
    cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
	cout << "::                                                 ::" << endl;
	cout << "::              wzk's compiler  V1.0               ::" << endl;
	cout << "::                                                 ::" << endl;
	cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;

	//语法分析、错误处理
	Grammar grammar("testfile.txt");
    grammar.analyze();
    grammar.save_to_file("output.txt");
    Errors::save_to_file("error.txt");


    //中间代码优化

    PseudoCodeList::refactor();



    PseudoCodeList::save_to_file("pseudo_code_old.txt");

//    string fileid = "1";
//    PseudoCodeList::save_to_file("docs/codes/testfile" + fileid + "_18231047_王肇凯_优化前中间代码.txt");
//    MipsGenerator mips_old;
//    mips_old.translate();
//    mips_old.save_to_file("docs/codes/testfile" + fileid + "_18231047_王肇凯_优化前目标代码.txt");

    PseudoCodeList::remove_redundant_assign();
    PseudoCodeList::const_broadcast();
    PseudoCodeList::remove_redundant_tmp();

    PseudoCodeList::divide_basic_blocks();
    //PseudoCodeList::DAG_optimize();

    //PseudoCodeList::const_broadcast();




    PseudoCodeList::inline_function();
    PseudoCodeList::const_broadcast();

    //PseudoCodeList::save_to_file("docs/codes/testfile" + fileid + "_18231047_王肇凯_优化后中间代码.txt");
    PseudoCodeList::save_to_file("pseudo_code.txt");

    //目标代码生成
    MipsGenerator mips;
    mips.optimize_muldiv = true;
    mips.optimize_assign_reg = true;
    mips.translate();
//    mips.save_to_file("docs/codes/testfile" + fileid + "_18231047_王肇凯_优化后目标代码.txt");
    mips.save_to_file("mips.txt");

    //SymTable::show();

    //grammar.show_tree();

    if (Errors::terminate()) {
        return 0;
    }

    return 0;
}
