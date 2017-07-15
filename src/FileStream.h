#pragma once

#include <string>
#include <cstdio>

#include "InputStream.h"

using namespace std;

struct FileStream : InputStream
{
  FileStream() = default;

  ~FileStream()
  {
    close();
  }

  void open(string filename)
  {
    m_fp = fopen(filename.c_str(), "rb");

    if(!m_fp)
      throw runtime_error("Can't open '" + filename + "' for reading");
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

  // disable copy
  FileStream(FileStream const &) = delete;
  FileStream & operator = (FileStream const &) = delete;

private:
  FILE* m_fp = nullptr;
};

#include "OutputStream.h"

struct OutputFileStream : OutputStream
{
  OutputFileStream() = default;

  ~OutputFileStream()
  {
    if(!m_fp)
      return;

    fclose(m_fp);
    m_fp = NULL;
  }

  void open(string filename)
  {
    m_fp = fopen(filename.c_str(), "wb");

    if(!m_fp)
      throw runtime_error("Can't open '" + filename + "' for writing");
  }

  virtual void write(const uint8_t* dst, size_t len)
  {
    auto const size = fwrite(dst, 1, len, m_fp);

    if(size != len)
      throw runtime_error("write failed");
  }

  // disable copy
  OutputFileStream(OutputFileStream const &) = delete;
  OutputFileStream & operator = (OutputFileStream const &) = delete;

private:
  FILE* m_fp = nullptr;
};

