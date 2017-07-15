#include <cassert>
#include <iostream>
#include "ClassFile.h"

string getString(ClassFile const& class_, int index)
{
  auto& constant = class_.const_pool_[index];
  assert(constant.tag_ == (int)CONSTANT::Utf8);
  return constant.utf8_;
}

string getClass(ClassFile const& class_, int index)
{
  auto& constant = class_.const_pool_[index];
  assert(constant.tag_ == (int)CONSTANT::Class);
  return getString(class_, constant.name_index_);
}

void dumpClass(ClassFile const& class_)
{
  if(0)
  {
    int i = 0;
    cout << "Constants:" << endl;

    for(auto& constant : class_.const_pool_)
    {
      (void)constant;
      cout << "[" << i << "] ";

      if(constant.tag_ == (int)CONSTANT::Utf8)
        cout << constant.utf8_;

      cout << endl;

      ++i;
    }
  }

  cout << "Class: " << getClass(class_, class_.this_class_) << endl;

  if(0)
  {
    for(auto& method : class_.methods_)
      cout << " * method: " << getString(class_, method.name_index_) << endl;

    cout << endl;
  }
}

