#pragma once

#include <map>
#include <string>
#include "ClassFile.h"

using namespace std;

struct SourceId
{
  bool isMethod; // otherwise, field
  string className;
  string name;

  bool operator < (SourceId const& other) const
  {
    if(isMethod < other.isMethod)
      return true;

    if(isMethod > other.isMethod)
      return false;

    if(className < other.className)
      return true;

    if(className > other.className)
      return false;

    return name < other.name;
  }
};

extern map<string, string> classRemap;
extern map<SourceId, string> memberRemap;

void doRenamings(ClassFile& class_);

