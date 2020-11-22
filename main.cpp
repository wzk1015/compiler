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

//    MidCodeList::reset();
//    SymTable::reset();
//    Grammar optimizer("testfile_replace.txt", gen_inline);
//    optimizer.analyze();
//    optimizer.save_lexer_results("testfile_optimize.txt");
//
//    //语义分析、中间代码生成
//    MidCodeList::reset();
//    SymTable::reset();
//    Grammar semantic("testfile_optimize.txt", semantic_analyze);
//    semantic.analyze();

    //中间代码优化
    MidCodeList::refactor();
    MidCodeList::remove_redundant_assign();
    MidCodeList::save_to_file("midCode_old.txt");
    MidCodeList::const_broadcast();
    MidCodeList::save_to_file("midCode.txt");

    //目标代码生成
    MipsGenerator mips;
    mips.translate();
    mips.save_to_file("mips.txt");

    SymTable::show();

    //grammar.show_tree();
    return 0;
}
