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

void remapClassReferences(ClassFile& class_)
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
      remapFieldDescriptor(typeNameEntry);
    }
  }

  for(auto& field : class_.fields)
  {
    auto& desc = class_.const_pool[field.descriptor_index];
    remapFieldDescriptor(desc);
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

  while(!fp.eof())
  {
    string type, from, to;
    fp >> type >> ws >> from >> to;

    if(type == "class")
      classRemap[from] = to;
  }

  for(auto& remapping : classRemap)
  {
    cout << remapping.first << " -> " << remapping.second << endl;
  }
}

struct Config
{
  string inputPath, outputPath;
  string remapPath;
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
    else
      throw runtime_error("Unknown switch: '" + word + "'");
  }

  return config;
}

void processOneClass(ClassFile& class_)
{
  remapClassReferences(class_);
  dumpClass(class_);
}

int main(int argc, char** argv)
{
  try
  {
    auto config = parseCommandLine(argc, argv);

    if(config.inputPath.empty())
      throw runtime_error("no input path specified");

    if(!config.remapPath.empty())
      loadRemappings(config.remapPath);

    if(endsWith(config.inputPath, ".jar"))
    {
      JarFile jar(config.inputPath);

      if(!config.remapPath.empty())
      {
        auto& classes = jar.getAllClasses();

        for(auto& class_ : classes)
          processOneClass(class_.second);

        renameFiles(classes);
      }

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

      if(!config.remapPath.empty())
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

