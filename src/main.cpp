#include <iostream>
#include <stdexcept>

#include <string>
#include "FileStream.h"
#include "ClassFile.h"
#include "InputStream.h"
#include "JarFile.h"

using namespace std;

// module entry point
ClassFile parseClass(InputStream* fp);

int main(int argc, char** argv)
{
  try
  {
    for(int i = 1; i < argc; ++i)
    {
      if(endsWith(argv[i], ".jar"))
      {
        JarFile jar(argv[i]);
      }
      else
      {
        FileStream fp;
        fp.open(argv[i]);

        auto class_ = parseClass(&fp);

        if(!fp.eof())
          throw runtime_error("parse error");

        cout << "Class has " << class_.methods_.size() << " methods" << endl;
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

