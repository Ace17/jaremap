#include <cassert>
#include <iostream>
#include <stdexcept>

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

void renameClasses(ClassFile& class_)
{
  for(auto& constant : class_.const_pool_)
  {
    if(constant.tag_ != (int)CONSTANT::Class)
      continue;

    auto& classNameEntry = class_.const_pool_[constant.name_index_];

    auto const& origClassName = classNameEntry.utf8_;
    auto i_newName = classRemap.find(origClassName);

    if(i_newName == classRemap.end())
      continue;

    classNameEntry.utf8_ = i_newName->second;
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

int main(int argc, char** argv)
{
  classRemap["we"] = "WeekEnd";

  try
  {
    for(int i = 1; i < argc; ++i)
    {
      if(endsWith(argv[i], ".jar"))
      {
        JarFile jar(argv[i]);

        auto& classes = jar.getAllClasses();

        for(auto& class_ : classes)
          renameClasses(class_.second);

        renameFiles(classes);

        jar.save("serialized.jar");
      }
      else
      {
        FileStream fp;
        fp.open(argv[i]);

        auto class_ = parseClass(&fp);

        if(!fp.eof())
          throw runtime_error("parse error");

        dumpClass(class_);

        renameClasses(class_);

        OutputFileStream out;
        out.open("serialized.class");
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

