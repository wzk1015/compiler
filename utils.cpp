//
// Created by wzk on 2020/10/24.
//

#include "utils.h"

string lower(string wd) {
    string s;
    int len = wd.size();
    for (int i = 0; i < len; i++) {
        if (wd[i] >= 'A' && wd[i] <= 'Z') {
            s += (char) (wd[i] + 'a' - 'A');
        } else {
            s += wd[i];
        }
    }
    return s;
}

bool is_2_power(int x){
    return (x&(x-1))==0;
}