#include <stdexcept>
#include "ClassFile.h"
#include "OutputStream.h"

namespace
{
void writeUnsigned(OutputStream* fp, int n, uint32_t val)
{
  uint8_t buf[16];

  for(int i = n; i >= 0; --i)
    buf[i] = (val >> (8 * (n - i - 1))) & 0xff;

  fp->write(buf, n);
}

void writeWord(OutputStream* fp, uint16_t val)
{
  writeUnsigned(fp, 2, val);
}

struct Writer
{
  Writer(ClassFile const& c) : m_class(c)
  {
  }

  ClassFile const& m_class;

  void write(OutputStream* fp)
  {
    writeHeader(fp);
    writeConstPool(fp);
    writeFlagAndClass(fp);
    writeInterface(fp);
    writeField(fp);
    writeMethod(fp);
    writeAttr(fp);
  }

  void writeHeader(OutputStream* fp)
  {
    writeUnsigned(fp, 4, 0xCAFEBABE);
    writeWord(fp, m_class.minor_version);
    writeWord(fp, m_class.major_version);
  }

  void writeConstPool(OutputStream* fp)
  {
    writeWord(fp, m_class.const_pool_count);

    for(size_t i = 1; i < m_class.const_pool_count; ++i)   // no use index 0
    {
      writeUnsigned(fp, 1, (int)m_class.const_pool[i].tag);

      writeConstPoolContents(m_class.const_pool[i], fp);

      // those are take two entries
      if(m_class.const_pool[i].tag == CONSTANT::Long
         || m_class.const_pool[i].tag == CONSTANT::Double)
        ++i;
    }
  }

  void writeConstPoolContents(const ConstPoolInfo& info, OutputStream* fp)
  {
    switch(info.tag)
    {
    case CONSTANT::Fieldref:
    case CONSTANT::Methodref:
    case CONSTANT::InterfaceMethodref:
      writeWord(fp, info.class_index);
      writeWord(fp, info.name_and_type_index);
      break;
    case CONSTANT::InvokeDynamic:
      fp->write(info.info, 2);
      fp->write(&info.info[2], 2);
      break;
    case CONSTANT::NameAndType:
      writeWord(fp, info.name_index);
      writeWord(fp, info.descriptor_index);
      break;
    case CONSTANT::Class:
      writeWord(fp, info.name_index);
      break;
    case CONSTANT::String:
    case CONSTANT::MethodType:
      fp->write(info.info, 2);
      break;
    case CONSTANT::Integer:
    case CONSTANT::Float:
      fp->write(info.info, 4);
      break;
    case CONSTANT::Long:
    case CONSTANT::Double:
      fp->write(info.info, 4);
      fp->write(&info.info[4], 4);
      break;
    case CONSTANT::Utf8:
      {
        auto const len = info.utf8.size();
        writeWord(fp, len);
        fp->write((uint8_t*)info.utf8.data(), len);
      }
      break;
    case CONSTANT::MethodHandle:
      fp->write(info.info, 1);
      fp->write(&info.info[1], 2);
      break;
    default:
      throw runtime_error("unsupported constant pool tag");
    }
  }

  void writeFlagAndClass(OutputStream* fp)
  {
    writeWord(fp, m_class.access_flags);
    writeWord(fp, m_class.this_class);
    writeWord(fp, m_class.super_class);
  }

  void writeInterface(OutputStream* fp)
  {
    writeWord(fp, m_class.interfaces.size());

    for(auto& itf : m_class.interfaces)
      writeWord(fp, itf);
  }

  void writeField(OutputStream* fp)
  {
    writeWord(fp, m_class.fields.size());

    for(auto& field : m_class.fields)
    {
      writeWord(fp, field.access_flags);
      writeWord(fp, field.name_index);
      writeWord(fp, field.descriptor_index);
      writeWord(fp, field.attrs_count);

      writeAttrInner(field.attrs, field.attrs_count, fp);
    }
  }

  void writeMethod(OutputStream* fp)
  {
    writeWord(fp, m_class.methods.size());

    for(auto& method : m_class.methods)
    {
      writeWord(fp, method.access_flags);
      writeWord(fp, method.name_index);
      writeWord(fp, method.descriptor_index);
      writeWord(fp, method.attrs_count);

      writeAttrInner(method.attrs, method.attrs_count, fp);
    }
  }

  void writeAttr(OutputStream* fp)
  {
    writeWord(fp, m_class.attrs_count);
    writeAttrInner(m_class.attrs, m_class.attrs_count, fp);
  }

  void writeAttrInner(const vector<AttrInfo>& info, size_t num, OutputStream* fp)
  {
    for(auto& inf : info)
    {
      writeWord(fp, inf.attr_name_index);
      writeUnsigned(fp, 4, inf.attr_len);

      writeAttrInnerInner(inf.info, inf.attr_len, fp);
    }
  }

  void writeAttrInnerInner(const vector<uint8_t>& info, size_t num, OutputStream* fp)
  {
    for(auto& inf : info)
      fp->write(&inf, 1);
  }
};
}

// module entry point
void writeClass(OutputStream* fp, ClassFile const& class_)
{
  Writer p(class_);
  p.write(fp);
}

