#include "ClassFile.h"
#include <iostream>
#include <cassert>

namespace
{
string getString(ClassFile const& class_, int index)
{
  auto& constant = class_.const_pool[index];
  assert(constant.tag == CONSTANT::Utf8);
  return constant.utf8;
}

string getClassName(ClassFile const& class_, int index)
{
  auto& constant = class_.const_pool[index];
  assert(constant.tag == CONSTANT::Class);
  return getString(class_, constant.name_index);
}
}

void printClass(ClassFile const& class_)
{
  auto me = getClassName(class_, class_.this_class);
  cout << "Class: " << me << endl;
  cout << "Uses:" << endl;

  for(auto& entry : class_.const_pool)
  {
    switch(entry.tag)
    {
    case CONSTANT::Fieldref:
    case CONSTANT::Methodref:
    case CONSTANT::InterfaceMethodref:
      {
        cout << " * ";
        cout << getClassName(class_, entry.class_index) << ".";
        auto sig = class_.const_pool[entry.name_and_type_index];
        cout << getString(class_, sig.name_index) << " [" << getString(class_, sig.descriptor_index) << "]" << endl;
        break;
      }
    default:
      break;
    }
  }

  cout << "Defines:" << endl;

  for(auto& entry : class_.methods)
    cout << " * " << me << "." << getString(class_, entry.name_index) << " [" << getString(class_, entry.descriptor_index) << "]" << endl;

  for(auto& entry : class_.fields)
    cout << " * " << me << "." << getString(class_, entry.name_index) << " [" << getString(class_, entry.descriptor_index) << "]" << endl;
}

