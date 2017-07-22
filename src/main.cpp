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
#include "Renamer.h"

using namespace std;

ClassFile parseClass(InputStream* fp);
void writeClass(OutputStream* fp, ClassFile const& class_);
void dumpClass(ClassFile const& class_);
void printClass(ClassFile const& class_);

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
    string type;
    iss >> type;

    if(type == "class")
    {
      string from, to;
      iss >> ws >> from >> to;
      classRemap[from] = to;
    }
    else if(type == "field")
    {
      string class_, from, to;
      iss >> ws >> class_ >> ws >> from >> to;
      memberRemap[{ false, class_, from }] = to;
    }
    else if(type == "method")
    {
      string class_, from, to;
      iss >> ws >> class_ >> ws >> from >> to;
      memberRemap[{ true, class_, from }] = to;
    }
  }
}

struct Config
{
  string inputPath, outputPath;
  string remapPath;
  bool verbose = false;
  bool print = false;
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
    else if(word == "-p")
      config.print = true;
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

        if(config.print)
          printClass(class_);
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

