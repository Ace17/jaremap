#include <stdexcept>
#include "ClassFile.h"
#include "OutputStream.h"

namespace
{
void writeUnsigned(OutputStream* fp, int n, uint32_t val)
{
  uint8_t buf[16];

  for(int i = n; i >= 0; --i)
    buf[i] = (val >> (8 * (n-i-1))) & 0xff;

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
    writeWord(fp, m_class.minor_version_);
    writeWord(fp, m_class.major_version_);
  }

  void writeConstPool(OutputStream* fp)
  {
    writeWord(fp, m_class.const_pool_count_);

    for(size_t i = 1; i < m_class.const_pool_count_; ++i)   // no use index 0
    {
      writeUnsigned(fp, 1, m_class.const_pool_[i].tag_);

      writeConstPoolContents(m_class.const_pool_[i], fp);

      // those are take two entries
      if((CONSTANT)m_class.const_pool_[i].tag_ == CONSTANT::Long
         || (CONSTANT)m_class.const_pool_[i].tag_ == CONSTANT::Double)
        ++i;
    }
  }

  void writeConstPoolContents(const ConstPoolInfo& info, OutputStream* fp)
  {
    switch((CONSTANT)info.tag_)
    {
    case CONSTANT::Fieldref:
    case CONSTANT::Methodref:
    case CONSTANT::InterfaceMethodref:
    case CONSTANT::NameAndType:
    case CONSTANT::InvokeDynamic:
      fp->write(info.info_, 2);
      fp->write(&info.info_[2], 2);
      break;
    case CONSTANT::String:
    case CONSTANT::Class:
    case CONSTANT::MethodType:
      fp->write(info.info_, 2);
      break;
    case CONSTANT::Integer:
    case CONSTANT::Float:
      fp->write(info.info_, 4);
      break;
    case CONSTANT::Long:
    case CONSTANT::Double:
      fp->write(info.info_, 4);
      fp->write(&info.info_[4], 4);
      break;
    case CONSTANT::Utf8:
      {
        fp->write(info.info_, 2);

        auto const len = info.info_[0] * 256 + info.info_[1];

        fp->write(info.bytes_.data(), len);
      }
      break;
    case CONSTANT::MethodHandle:
      fp->write(info.info_, 1);
      fp->write(&info.info_[1], 2);
      break;
    default:
      throw runtime_error("unsupported constant pool tag");
    }
  }

  void writeFlagAndClass(OutputStream* fp)
  {
    writeWord(fp, m_class.access_flags_);
    writeWord(fp, m_class.this_class_);
    writeWord(fp, m_class.super_class_);
  }

  void writeInterface(OutputStream* fp)
  {
    writeWord(fp, m_class.interfaces_.size());

    for(auto& itf : m_class.interfaces_)
      writeWord(fp, itf);
  }

  void writeField(OutputStream* fp)
  {
    writeWord(fp, m_class.fields_.size());

    for(auto& field : m_class.fields_)
    {
      writeWord(fp, field.access_flags_);
      writeWord(fp, field.name_index_);
      writeWord(fp, field.descriptor_index_);
      writeWord(fp, field.attrs_count_);

      writeAttrInner(field.attrs_, field.attrs_count_, fp);
    }
  }

  void writeMethod(OutputStream* fp)
  {
    writeWord(fp, m_class.methods_.size());

    for(auto& method : m_class.methods_)
    {
      writeWord(fp, method.access_flags_);
      writeWord(fp, method.name_index_);
      writeWord(fp, method.descriptor_index_);
      writeWord(fp, method.attrs_count_);

      writeAttrInner(method.attrs_, method.attrs_count_, fp);
    }
  }

  void writeAttr(OutputStream* fp)
  {
    writeWord(fp, m_class.attrs_count_);
    writeAttrInner(m_class.attrs_, m_class.attrs_count_, fp);
  }

  void writeAttrInner(const vector<AttrInfo>& info, size_t num, OutputStream* fp)
  {
    for(auto& inf : info)
    {
      writeWord(fp, inf.attr_name_index_);
      writeUnsigned(fp, 4, inf.attr_len_);

      writeAttrInnerInner(inf.info_, inf.attr_len_, fp);
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

