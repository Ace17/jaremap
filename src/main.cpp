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

int main(int argc, char** argv)
{
  try
  {
    for(int i = 1; i < argc; ++i)
    {
      if(endsWith(argv[i], ".jar"))
      {
        JarFile jar(argv[i]);

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

