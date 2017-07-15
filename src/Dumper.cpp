#include <cassert>
#include <iostream>
#include "ClassFile.h"

string getString(ClassFile const& class_, int index)
{
  auto& constant = class_.const_pool_[index];
  assert(constant.tag_ == (int)CONSTANT::Utf8);
  return string(constant.bytes_.begin(), constant.bytes_.end());
}

void dumpClass(ClassFile const& class_)
{
  cout << "Class" << endl;

  if(0)
  {
    int i = 0;
    cout << "Constants:" << endl;

    for(auto& constant : class_.const_pool_)
    {
      (void)constant;
      cout << "[" << i << "] ";

      if(constant.tag_ == (int)CONSTANT::Utf8)
        cout << constant.bytes_.data();

      cout << endl;

      ++i;
    }
  }

  for(auto& method : class_.methods_)
  {
    (void)method;
    cout << " * method: " << getString(class_, method.name_index_) << endl;
  }

  cout << endl;
}

