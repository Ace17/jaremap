#include <stdexcept>
#include "ClassFile.h"
#include "InputStream.h"

namespace
{
uint32_t readUnsigned(InputStream* fp, int n)
{
  uint8_t buf[256];

  fp->read(buf, n);

  uint32_t val = 0;

  for(int i = 0; i < n; ++i)
  {
    val <<= 8;
    val |= buf[i];
  }

  return val;
}

uint16_t readWord(InputStream* fp)
{
  return readUnsigned(fp, 2);
}

struct Parser
{
  ClassFile m_class;

  void parse(InputStream* fp)
  {
    parseHeader(fp);
    parseConstPool(fp);
    parseFlagAndClass(fp);
    parseInterface(fp);
    parseField(fp);
    parseMethod(fp);
    parseAttr(fp);
  }

  void parseHeader(InputStream* fp)
  {
    auto const magic = readUnsigned(fp, 4);

    if(magic != 0xCAFEBABE)
      throw runtime_error("invalid magic");

    m_class.minor_version = readWord(fp);
    m_class.major_version = readWord(fp);
  }

  void parseConstPool(InputStream* fp)
  {
    m_class.const_pool_count = readWord(fp);

    m_class.const_pool.resize(m_class.const_pool_count);

    for(size_t i = 1; i < m_class.const_pool_count; ++i)   // no use index 0
    {
      uint8_t tag = readUnsigned(fp, 1);

      parseConstPoolContents(m_class.const_pool[i], fp, tag);

      // those take two entries
      if((CONSTANT)tag == CONSTANT::Long || (CONSTANT)tag == CONSTANT::Double)
        ++i;
    }
  }

  void parseConstPoolContents(ConstPoolInfo& info, InputStream* fp, uint8_t tag)
  {
    info.tag = (CONSTANT)tag;
    switch(info.tag)
    {
    case CONSTANT::Fieldref:
    case CONSTANT::Methodref:
    case CONSTANT::InterfaceMethodref:
      info.class_index = readWord(fp);
      info.name_and_type_index = readWord(fp);
      break;
    case CONSTANT::InvokeDynamic:
      fp->read(info.info, 2);
      fp->read(&info.info[2], 2);
      break;
    case CONSTANT::NameAndType:
      info.name_index = readWord(fp);
      info.descriptor_index = readWord(fp);
      break;
    case CONSTANT::Class:
      info.name_index = readWord(fp);
      break;
    case CONSTANT::String:
    case CONSTANT::MethodType:
      fp->read(info.info, 2);
      break;
    case CONSTANT::Integer:
    case CONSTANT::Float:
      fp->read(info.info, 4);
      break;
    case CONSTANT::Long:
    case CONSTANT::Double:
      fp->read(info.info, 4);
      fp->read(&info.info[4], 4);
      break;
    case CONSTANT::Utf8:
      {
        auto const len = readWord(fp);
        info.utf8.resize(len);
        fp->read((uint8_t*)info.utf8.data(), len);
      }
      break;
    case CONSTANT::MethodHandle:
      fp->read(info.info, 1);
      fp->read(&info.info[1], 2);
      break;
    default:
      throw runtime_error("unsupported constant pool tag");
    }
  }

  void parseFlagAndClass(InputStream* fp)
  {
    m_class.access_flags = readWord(fp);
    m_class.this_class = readWord(fp);
    m_class.super_class = readWord(fp);
  }

  void parseInterface(InputStream* fp)
  {
    auto const interfaceCount = readWord(fp);
    m_class.interfaces.resize(interfaceCount);

    for(auto& itf : m_class.interfaces)
      itf = readWord(fp);
  }

  void parseField(InputStream* fp)
  {
    auto const fieldCount = readWord(fp);
    m_class.fields.resize(fieldCount);

    for(auto& field : m_class.fields)
    {
      field.access_flags = readWord(fp);
      field.name_index = readWord(fp);
      field.descriptor_index = readWord(fp);

      parseAttrInner(field.attrs, fp);
    }
  }

  void parseMethod(InputStream* fp)
  {
    auto const methodCount = readWord(fp);
    m_class.methods.resize(methodCount);

    for(auto& method : m_class.methods)
    {
      method.access_flags = readWord(fp);
      method.name_index = readWord(fp);
      method.descriptor_index = readWord(fp);

      parseAttrInner(method.attrs, fp);
    }
  }

  void parseAttr(InputStream* fp)
  {
    parseAttrInner(m_class.attrs, fp);
  }

  void parseAttrInner(vector<AttrInfo>& attributes, InputStream* fp)
  {
    attributes.resize(readWord(fp));

    for(auto& attr : attributes)
    {
      attr.attr_name_index = readWord(fp);
      attr.attr_len = readUnsigned(fp, 4);

      attr.info.resize(attr.attr_len);

      for(auto& inf : attr.info)
        fp->read(&inf, 1);
    }
  }
};
}

// module entry point
ClassFile parseClass(InputStream* fp)
{
  Parser p;
  p.parse(fp);

  return p.m_class;
}

