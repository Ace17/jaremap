#include <cassert>
#include <iostream>
#include <fstream>

#include <string>
#include <map>
#include "ClassFile.h"

using namespace std;

map<string, string> classRemap;

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
  if(from.empty())
    return;

  size_t i = 0;

  while((i = str.find(from, i)) != std::string::npos)
  {
    str.replace(i, from.length(), to);
    i += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

void remapDescriptorQuickAndDirty(ConstPoolInfo& desc)
{
  assert(desc.tag == CONSTANT::Utf8);

  for(auto& remapping : classRemap)
  {
    auto const from = "L" + remapping.first + ";";
    auto const to = "L" + remapping.second + ";";
    replaceAll(desc.utf8, from, to);
  }
}

void remapFieldDescriptor(ConstPoolInfo& desc)
{
  assert(desc.tag == CONSTANT::Utf8);

  if(desc.utf8[0] == 'L')
  {
    assert(desc.utf8.back() == ';');

    auto const origClassName = desc.utf8.substr(1, desc.utf8.size() - 2);
    auto i_newName = classRemap.find(origClassName);

    if(i_newName != classRemap.end())
      desc.utf8 = "L" + i_newName->second + ";";
  }
}

void doRenamings(ClassFile& class_)
{
  for(auto& constant : class_.const_pool)
  {
    if(constant.tag == CONSTANT::Class)
    {
      auto& classNameEntry = class_.const_pool[constant.name_index];

      auto const& origClassName = classNameEntry.utf8;
      auto i_newName = classRemap.find(origClassName);

      if(i_newName != classRemap.end())
        classNameEntry.utf8 = i_newName->second;
    }
    else if(constant.tag == CONSTANT::NameAndType)
    {
      auto& typeNameEntry = class_.const_pool[constant.descriptor_index];
      remapDescriptorQuickAndDirty(typeNameEntry);
    }
  }

  for(auto& field : class_.fields)
  {
    auto& desc = class_.const_pool[field.descriptor_index];
    remapFieldDescriptor(desc);
  }

  for(auto& method : class_.methods)
  {
    auto& desc = class_.const_pool[method.descriptor_index];
    remapDescriptorQuickAndDirty(desc);
  }
}

