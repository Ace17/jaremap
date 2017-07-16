#include <cassert>
#include <iostream>
#include <stdexcept>
#include <fstream>

#include <string>
#include "FileStream.h"
#include "ClassFile.h"
#include "InputStream.h"
#include "OutputStream.h"
#include "JarFile.h"

using namespace std;

ClassFile parseClass(InputStream* fp);
void writeClass(OutputStream* fp, ClassFile const& class_);
void dumpClass(ClassFile const& class_);

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

void renameFiles(map<string, ClassFile>& files)
{
  for(auto& remapping : classRemap)
  {
    auto const from = remapping.first;
    auto const to = remapping.second;
    auto i_file = files.find(from);

    if(i_file == files.end())
    {
      cerr << "Available classes: ";

      for(auto& file : files)
        cerr << file.first << " ";

      cerr << endl;
      throw runtime_error("Class '" + from + "' doesn't exist");
    }

    auto file = move(i_file->second);
    files.erase(i_file);

    files[to] = move(file);
  }
}

void loadRemappings(string path)
{
  ifstream fp(path);

  if(!fp.is_open())
    throw runtime_error("can't open '" + path + "' for reading");

  string line;

  while(getline(fp, line))
  {
    istringstream iss(line);
    string type, from, to;
    iss >> type >> ws >> from >> to;

    if(type == "class")
      classRemap[from] = to;
  }
}

struct Config
{
  string inputPath, outputPath;
  string remapPath;
  bool verbose = false;
};

Config parseCommandLine(int argc, char** argv)
{
  Config config;
  int k = 1;
  auto moreArgs = [&] () -> bool { return k < argc; };
  auto popArg =
    [&] () -> string {
      if(!moreArgs())
        throw runtime_error("unepxected end of command line");

      return argv[k++];
    };

  while(moreArgs())
  {
    auto word = popArg();

    if(word == "-r")
      config.remapPath = popArg();
    else if(word == "-i")
      config.inputPath = popArg();
    else if(word == "-o")
      config.outputPath = popArg();
    else if(word == "-v")
      config.verbose = true;
    else
      throw runtime_error("Unknown switch: '" + word + "'");
  }

  return config;
}

int main(int argc, char** argv)
{
  try
  {
    auto config = parseCommandLine(argc, argv);

    if(config.inputPath.empty())
      throw runtime_error("no input path specified");

    if(!config.remapPath.empty())
    {
      loadRemappings(config.remapPath);

      if(config.verbose)
      {
        for(auto& remapping : classRemap)
          cout << remapping.first << " -> " << remapping.second << endl;
      }
    }

    auto processOneClass =
      [&] (ClassFile& class_)
      {
        if(!config.remapPath.empty())
          doRenamings(class_);

        if(config.verbose)
          dumpClass(class_);
      };

    if(endsWith(config.inputPath, ".jar"))
    {
      JarFile jar(config.inputPath);

      auto& classes = jar.getAllClasses();

      for(auto& class_ : classes)
        processOneClass(class_.second);

      if(!config.remapPath.empty())
        renameFiles(classes);

      if(!config.outputPath.empty())
        jar.save(config.outputPath);
    }
    else
    {
      FileStream fp;
      fp.open(config.inputPath);

      auto class_ = parseClass(&fp);

      if(!fp.eof())
        throw runtime_error("parse error");

      processOneClass(class_);

      if(!config.outputPath.empty())
      {
        OutputFileStream out;
        out.open(config.outputPath);
        writeClass(&out, class_);
      }
    }

    return 0;
  }
  catch(runtime_error const& e)
  {
    cerr << "Fatal: " << e.what() << endl;
    return 1;
  }
}

