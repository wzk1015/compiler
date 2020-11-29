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
	Grammar grammar("testfile.txt", grammar_check);
    grammar.analyze();
    grammar.save_to_file("output.txt");
//    grammar.save_lexer_results("testfile_replace.txt");
    Errors::save_to_file("error.txt");

    if (Errors::terminate()) {
        return 0;
    }

//    PseudoCodeList::reset();
//    SymTable::reset();
//    Grammar optimizer("testfile_replace.txt", gen_inline);
//    optimizer.analyze();
//    optimizer.save_lexer_results("testfile_optimize.txt");
//
//    //语义分析、中间代码生成
//    PseudoCodeList::reset();
//    SymTable::reset();
//    Grammar semantic("testfile_optimize.txt", semantic_analyze);
//    semantic.analyze();

    //中间代码优化

    PseudoCodeList::refactor();

    PseudoCodeList::remove_redundant_assign();
    PseudoCodeList::const_broadcast();
    PseudoCodeList::remove_redundant_tmp();

    PseudoCodeList::divide_basic_blocks();

    PseudoCodeList::save_to_file("pseudoCode_old.txt");

    //PseudoCodeList::DAG_optimize();

    //PseudoCodeList::const_broadcast();

    PseudoCodeList::save_to_file("pseudoCode.txt");

    //目标代码生成
    MipsGenerator mips;
    mips.translate();
    mips.save_to_file("mips.txt");
//
//    SymTable::show();

    //grammar.show_tree();
    return 0;
}
