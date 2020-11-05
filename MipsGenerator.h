//
// Created by wzk on 2020/11/5.
//

#ifndef COMPILER_MIPSGENERATOR_H
#define COMPILER_MIPSGENERATOR_H

#include <utility>

#include "MidCode.h"

class MipsGenerator {
    vector<MidCode> mid;
    vector<string> mips;

    explicit MipsGenerator(vector<MidCode> codes): mid(std::move(codes)) {};

    void generate();
};


#endif //COMPILER_MIPSGENERATOR_H
