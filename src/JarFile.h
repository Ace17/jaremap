#include "Archive.h"
#include "ClassFile.h"

#include <map>
#include <string>
#include <stdexcept>
#include <sstream>

ClassFile parseClass(InputStream* fp);

void writeClassFile(ClassFile const& class_, ostream& o)
{
  o << "Hello" << endl;
}

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
      if(!endsWith(fileName, ".class"))
        continue;

      auto contents = archive.getFile(fileName);
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

    for(auto class_ : m_classes)
    {
      auto const className = class_.first;
      ostringstream ss;
      writeClassFile(class_.second, ss);
      archive.setFile(className + ".class", ss.str());
    }
  }

  std::map<string, ClassFile> getAllClasses()
  {
    return m_classes;
  }

private:
  std::map<string, ClassFile> m_classes;
};

