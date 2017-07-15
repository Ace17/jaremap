#pragma once

#include <string>
#include <cstdio>

#include "InputStream.h"

using namespace std;

struct FileStream : InputStream
{
  FileStream() = default;
  FileStream(FileStream const &) = delete;
  FileStream & operator = (FileStream const &) = delete;

  ~FileStream()
  {
    close();
  }

  void open(string filename)
  {
    m_fp = fopen(filename.c_str(), "rb");

    if(!m_fp)
      throw runtime_error("Can't open '" + filename + "'");
  }

  void close()
  {
    if(!m_fp)
      return;

    fclose(m_fp);
    m_fp = NULL;
  }

  bool eof()
  {
    auto const curpos = ftell(m_fp);

    fseek(m_fp, 0, SEEK_END);

    if(ftell(m_fp) != curpos)
      return false;

    return true;
  }

  virtual void read(uint8_t* dst, size_t len)
  {
    auto const size = fread(dst, 1, len, m_fp);

    if(size != len)
      throw runtime_error("short read");
  }

private:
  FILE* m_fp = nullptr;
};

