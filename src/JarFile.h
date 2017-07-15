#include "Archive.h"
#include "ClassFile.h"

#include <map>
#include <string>
#include <stdexcept>
#include <sstream>

ClassFile parseClass(InputStream* fp);

void writeClass(OutputStream* fp, ClassFile const& class_);

using namespace std;

static inline
bool endsWith(const string& str, const string& suffix)
{
  if(str.size() < suffix.size())
    return false;

  return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

class JarFile
{
public:
  JarFile(string path)
  {
    load(path);
  }

  void load(string path)
  {
    Archive archive(path);

    for(auto fileName : archive.getFilenames())
    {
      auto contents = archive.getFile(fileName);

      if(!endsWith(fileName, ".class"))
      {
        m_opaqueFiles[fileName] = move(contents);
        continue;
      }

      auto const className = fileName.substr(0, fileName.size() - 6);

      struct BufferStream : InputStream
      {
        virtual void read(uint8_t* dst, size_t len)
        {
          for(size_t i = 0; i < len; ++i)
            dst[i] = (*data)[pos++];
        }

        string* data;
        size_t pos = 0;
      };

      BufferStream stream;
      stream.data = &contents;

      m_classes[className] = parseClass(&stream);

      if(stream.pos != contents.size())
        throw runtime_error("[jar] parse error");

      cout << "[jar] class has " << m_classes[className].methods_.size() << " methods" << endl;
    }
  }

  void save(string path)
  {
    Archive archive(path, true);

    for(auto& class_ : m_classes)
    {
      auto const className = class_.first;

      struct BufferStream : OutputStream
      {
        virtual void write(const uint8_t* dst, size_t len)
        {
          for(size_t i = 0; i < len; ++i)
            data += dst[i];
        }

        string data;
      };

      BufferStream stream;

      writeClass(&stream, class_.second);
      archive.setFile(className + ".class", stream.data);
    }

    for(auto& opaqueFile : m_opaqueFiles)
    {
      archive.setFile(opaqueFile.first, opaqueFile.second);
    }
  }

  std::map<string, ClassFile> getAllClasses()
  {
    return m_classes;
  }

private:
  std::map<string, ClassFile> m_classes;
  std::map<string, string> m_opaqueFiles;
};

