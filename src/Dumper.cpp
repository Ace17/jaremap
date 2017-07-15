#include <iostream>
#include "ClassFile.h"

void dumpClass(ClassFile const& class_)
{
  cout << "Class" << endl;
  for(auto& method : class_.methods_)
  {
    (void)method;
    cout << " * method" << endl;
  }
  cout << endl;
}

