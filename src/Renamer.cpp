#include <cassert>
#include <iostream>
#include <fstream>

#include <string>
#include <map>
#include "Renamer.h"
#include "ClassFile.h"

using namespace std;

map<string, string> classRemap;
map<SourceId, string> memberRemap;

namespace
{
string getString(ClassFile const& class_, int index)
{
  return class_.const_pool[index].utf8;
}

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

int addUtf8(ClassFile& class_, string text)
{
  auto const idx = (int)class_.const_pool.size();
  class_.const_pool_count++;
  class_.const_pool.push_back({ CONSTANT::Utf8, {}, text });
  return idx;
}

void renameClasses(ClassFile& class_)
{
  for(auto& constant : class_.const_pool)
  {
    if(constant.tag == CONSTANT::Class)
    {
      auto& classNameEntry = class_.const_pool[constant.name_index];

      auto const& origClassName = classNameEntry.utf8;
      auto i_newName = classRemap.find(origClassName);

      if(i_newName != classRemap.end())
        constant.name_index = addUtf8(class_, i_newName->second);
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

  // rename class names in method signatures
  for(auto& method : class_.methods)
  {
    auto& desc = class_.const_pool[method.descriptor_index];
    remapDescriptorQuickAndDirty(desc);
  }
}

void renameMembers(ClassFile& class_, map<SourceId, string> const& remap)
{
  // the other class members I'm referring to
  for(auto& constant : class_.const_pool)
  {
    if(constant.tag == CONSTANT::Fieldref || constant.tag == CONSTANT::Methodref)
    {
      auto const isMethod = constant.tag == CONSTANT::Methodref;
      auto const& className = getString(class_, class_.const_pool[constant.class_index].name_index);
      auto& nameAndType = class_.const_pool[constant.name_and_type_index];
      auto const& origName = getString(class_, nameAndType.name_index);

      auto i_newName = remap.find({ isMethod, className, origName });

      if(i_newName != remap.end())
        nameAndType.name_index = addUtf8(class_, i_newName->second);
    }
  }

  // the members I'm declaring
  for(auto& field : class_.fields)
  {
    auto const& className = getString(class_, class_.const_pool[class_.this_class].name_index);
    auto const& origName = getString(class_, field.name_index);

    auto i_newName = remap.find({ false, className, origName });

    if(i_newName != remap.end())
      field.name_index = addUtf8(class_, i_newName->second);
  }

  for(auto& method : class_.methods)
  {
    auto const& className = getString(class_, class_.const_pool[class_.this_class].name_index);
    auto const& fieldOrigName = getString(class_, method.name_index);

    auto i_newName = remap.find({ true, className, fieldOrigName });

    if(i_newName != remap.end())
      method.name_index = addUtf8(class_, i_newName->second);
  }
}
}

void doRenamings(ClassFile& class_)
{
  // HACK: avoid array re-allocation when adding new utf8 descriptors
  class_.const_pool.reserve(class_.const_pool.size() * 4);

  renameClasses(class_);
  renameMembers(class_, memberRemap);

  class_.const_pool.shrink_to_fit();
}

