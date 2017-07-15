#pragma once

#include <stdint.h>
#include <cstdio>
#include <vector>

using namespace std;

enum class CONSTANT
{
  Utf8 = 1,
  Integer = 3,
  Float = 4,
  Long = 5,
  Double = 6,
  Class = 7,
  String = 8,
  Fieldref = 9,
  Methodref = 10,
  InterfaceMethodref = 11,
  NameAndType = 12,
  MethodHandle = 15,
  MethodType = 16,
  InvokeDynamic = 18,
};

struct ConstPoolInfo
{
  uint8_t tag_ = 0;
  uint8_t info_[8] {};
  string utf8_;

  union
  {
    uint16_t name_index_;
  };
};

struct AttrInfo
{
  uint16_t attr_name_index_ = 0;
  uint32_t attr_len_ = 0;
  vector<uint8_t> info_;
};

struct FieldInfo
{
  uint16_t access_flags_ = 0;
  uint16_t name_index_ = 0;
  uint16_t descriptor_index_ = 0;
  uint16_t attrs_count_ = 0;
  vector<AttrInfo> attrs_;
};

struct MethodInfo : FieldInfo
{
};

struct ClassFile
{
  uint16_t minor_version_ = 0;
  uint16_t major_version_ = 0;
  uint16_t const_pool_count_ = 0;
  vector<ConstPoolInfo> const_pool_; // 1 ~ const_pool_count-1
  uint16_t access_flags_ = 0;
  uint16_t this_class_ = 0;
  uint16_t super_class_ = 0;
  vector<uint16_t> interfaces_;
  vector<FieldInfo> fields_;
  vector<MethodInfo> methods_;
  uint16_t attrs_count_ = 0;
  vector<AttrInfo> attrs_;
};

