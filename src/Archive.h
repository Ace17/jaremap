#pragma once

#include <vector>
#include <string>
#include <sstream>

using namespace std;

class Archive
{
public:
  Archive(string path, bool createNew = false);
  ~Archive();

  vector<string> getFilenames();
  string getFile(string path);
  void setFile(string path, string const& data);

private:
  void* m_handle;
};

