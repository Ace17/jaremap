#define ZIP_DISABLE_DEPRECATED
#include "Archive.h"
#include "zip.h"
#include <stdexcept>
#include <string.h> // memcpy

#define m_zip ((zip*)m_handle)

Archive::Archive(string path, bool createNew)
{
  if(createNew)
    remove(path.c_str());

  int error = 0;
  m_handle = zip_open(path.c_str(), createNew ? ZIP_CREATE : 0, &error);

  if(!m_handle)
    throw runtime_error("Can't open archive '" + path + "'");
}

Archive::~Archive()
{
  zip_close(m_zip);
}

string Archive::getFile(string path)
{
  auto file = zip_fopen(m_zip, path.c_str(), 0);

  if(!file)
    throw runtime_error("Can't find '" + path + "' inside archive");

  char buffer[1024];
  string contents;
  int bytes = 0;

  while((bytes = zip_fread(file, buffer, sizeof buffer)) > 0)
    contents += string(buffer, bytes);

  zip_fclose(file);
  return contents;
}

void Archive::setFile(string path, const string& data)
{
  auto ptr = static_cast<char*>(std::malloc(data.size()));

  if(!ptr)
    throw std::runtime_error("malloc failed");

  memcpy(ptr, data.data(), data.size());

  auto source = zip_source_buffer(m_zip, ptr, data.size(), 1);

  if(!source)
    throw runtime_error(string("Can't create zip source: ") + zip_strerror(m_zip));

  auto const idx = zip_file_add(m_zip, path.c_str(), source, 0);

  if(idx < 0)
    throw runtime_error("Can't create '" + path + "' inside archive: " + zip_strerror(m_zip));
}

vector<string> Archive::getFilenames()
{
  auto const N = zip_get_num_entries(m_zip, 0);

  vector<string> r;

  for(int i = 0; i < N; ++i)
    r.push_back(zip_get_name(m_zip, i, 0));

  return r;
}

