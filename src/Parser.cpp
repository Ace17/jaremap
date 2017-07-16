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

    m_class.minor_version_ = readWord(fp);
    m_class.major_version_ = readWord(fp);
  }

  void parseConstPool(InputStream* fp)
  {
    m_class.const_pool_count_ = readWord(fp);

    m_class.const_pool_.resize(m_class.const_pool_count_);

    for(size_t i = 1; i < m_class.const_pool_count_; ++i)   // no use index 0
    {
      uint8_t tag = readUnsigned(fp, 1);

      parseConstPoolContents(m_class.const_pool_[i], fp, tag);

      // those are take two entries
      if((CONSTANT)tag == CONSTANT::Long || (CONSTANT)tag == CONSTANT::Double)
        ++i;
    }
  }

  void parseConstPoolContents(ConstPoolInfo& info, InputStream* fp, uint8_t tag)
  {
    info.tag_ = (CONSTANT)tag;
    switch((CONSTANT)tag)
    {
    case CONSTANT::Fieldref:
    case CONSTANT::Methodref:
    case CONSTANT::InterfaceMethodref:
    case CONSTANT::InvokeDynamic:
      fp->read(info.info_, 2);
      fp->read(&info.info_[2], 2);
      break;
    case CONSTANT::NameAndType:
      info.name_index_ = readWord(fp);
      info.descriptor_index_ = readWord(fp);
      break;
    case CONSTANT::Class:
      info.name_index_ = readWord(fp);
      break;
    case CONSTANT::String:
    case CONSTANT::MethodType:
      fp->read(info.info_, 2);
      break;
    case CONSTANT::Integer:
    case CONSTANT::Float:
      fp->read(info.info_, 4);
      break;
    case CONSTANT::Long:
    case CONSTANT::Double:
      fp->read(info.info_, 4);
      fp->read(&info.info_[4], 4);
      break;
    case CONSTANT::Utf8:
      {
        auto const len = readWord(fp);
        info.utf8_.resize(len);
        fp->read((uint8_t*)info.utf8_.data(), len);
      }
      break;
    case CONSTANT::MethodHandle:
      fp->read(info.info_, 1);
      fp->read(&info.info_[1], 2);
      break;
    default:
      throw runtime_error("unsupported constant pool tag");
    }
  }

  void parseFlagAndClass(InputStream* fp)
  {
    m_class.access_flags_ = readWord(fp);
    m_class.this_class_ = readWord(fp);
    m_class.super_class_ = readWord(fp);
  }

  void parseInterface(InputStream* fp)
  {
    auto const interfaceCount = readWord(fp);
    m_class.interfaces_.resize(interfaceCount);

    for(auto& itf : m_class.interfaces_)
      itf = readWord(fp);
  }

  void parseField(InputStream* fp)
  {
    auto const fieldCount = readWord(fp);
    m_class.fields_.resize(fieldCount);

    for(auto& field : m_class.fields_)
    {
      field.access_flags_ = readWord(fp);
      field.name_index_ = readWord(fp);
      field.descriptor_index_ = readWord(fp);
      field.attrs_count_ = readWord(fp);

      parseAttrInner(field.attrs_, field.attrs_count_, fp);
    }
  }

  void parseMethod(InputStream* fp)
  {
    auto const methodCount = readWord(fp);
    m_class.methods_.resize(methodCount);

    for(auto& method : m_class.methods_)
    {
      method.access_flags_ = readWord(fp);
      method.name_index_ = readWord(fp);
      method.descriptor_index_ = readWord(fp);
      method.attrs_count_ = readWord(fp);

      parseAttrInner(method.attrs_, method.attrs_count_, fp);
    }
  }

  void parseAttr(InputStream* fp)
  {
    m_class.attrs_count_ = readWord(fp);

    parseAttrInner(m_class.attrs_, m_class.attrs_count_, fp);
  }

  void parseAttrInner(vector<AttrInfo>& info, size_t num, InputStream* fp)
  {
    info.resize(num);

    for(auto& inf : info)
    {
      inf.attr_name_index_ = readWord(fp);
      inf.attr_len_ = readUnsigned(fp, 4);

      parseAttrInnerInner(inf.info_, inf.attr_len_, fp);
    }
  }

  void parseAttrInnerInner(vector<uint8_t>& info, size_t num, InputStream* fp)
  {
    info.resize(num);

    for(auto& inf : info)
      fp->read(&inf, 1);
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

