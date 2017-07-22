#pragma once

#include <map>
#include <string>
#include "ClassFile.h"

using namespace std;

extern map<string, string> classRemap;
extern map<string, string> fieldRemap;
extern map<string, string> methodRemap;

void doRenamings(ClassFile& class_);

