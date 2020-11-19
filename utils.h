//
// Created by wzk on 2020/10/24.
//

#ifndef COMPILER_UTILS_H
#define COMPILER_UTILS_H

#include <iostream>
#include <vector>

using namespace std;

string lower(string);

int max(int, int);

int min(int, int);

bool is_2_power(int);

bool begins_num(string);

bool num_or_char(string);

void panic(const string&);

string str_replace(string str, const string& from, const string& to);

void assertion(bool);

int sum(const vector<int>& arr);

#endif //COMPILER_UTILS_H
