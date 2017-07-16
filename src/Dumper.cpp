#include <cassert>
#include <iostream>
#include "ClassFile.h"

string getString(ClassFile const& class_, int index)
{
  auto& constant = class_.const_pool[index];
  assert(constant.tag == CONSTANT::Utf8);
  return constant.utf8;
}

string getClass(ClassFile const& class_, int index)
{
  auto& constant = class_.const_pool[index];
  assert(constant.tag == CONSTANT::Class);
  return getString(class_, constant.name_index);
}

const char* TagToString(CONSTANT tag)
{
  switch(tag)
  {
  case CONSTANT::Utf8:  return "Utf8";
  case CONSTANT::Integer:  return "Integer";
  case CONSTANT::Float:  return "Float";
  case CONSTANT::Long:  return "Long";
  case CONSTANT::Double:  return "Double";
  case CONSTANT::Class:  return "Class";
  case CONSTANT::String:  return "String";
  case CONSTANT::Fieldref:  return "Fieldref";
  case CONSTANT::Methodref:  return "Methodref";
  case CONSTANT::InterfaceMethodref:  return "InterfaceMethodref";
  case CONSTANT::NameAndType:  return "NameAndType";
  case CONSTANT::MethodHandle:  return "MethodHandle";
  case CONSTANT::MethodType:  return "MethodType";
  case CONSTANT::InvokeDynamic:  return "InvokeDynamic";
  default: return "?";
  }
}

void dumpClass(ClassFile const& class_)
{
  if(1)
  {
    int i = 0;
    cout << "Constants:" << endl;

    for(auto& constant : class_.const_pool)
    {
      cout << " * " << i;
      cout << " [" << TagToString(constant.tag) << "] ";
      switch(constant.tag)
      {
      case CONSTANT::Utf8:
        cout << constant.utf8;
        break;
      case CONSTANT::Fieldref:
      case CONSTANT::Methodref:
      case CONSTANT::InterfaceMethodref:
        cout << "class=" << constant.class_index << " ";
        cout << "name_and_type=" << constant.name_and_type_index << " ";
        break;
      default:
        cout << "...";
        break;
      }

      cout << endl;

      ++i;
    }
  }

  cout << "Class: " << getClass(class_, class_.this_class) << endl;

  if(1)
  {
    for(auto& method : class_.methods)
      cout << " * method: " << getString(class_, method.name_index) << endl;

    cout << endl;
  }
}

