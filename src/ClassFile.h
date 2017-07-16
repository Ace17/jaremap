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
  CONSTANT tag {};
  uint8_t info[8] {};
  string utf8;

  union
  {
    struct
    {
      uint16_t name_index;
      uint16_t descriptor_index;
    };

    // Methodref, Fieldref, InterfaceMethodref
    struct
    {
      uint16_t class_index;
      uint16_t name_and_type_index;
    };
  };
};

struct AttrInfo
{
  uint16_t attr_name_index = 0;
  uint32_t attr_len = 0;
  vector<uint8_t> info;
};

struct FieldInfo
{
  uint16_t access_flags = 0;
  uint16_t name_index = 0;
  uint16_t descriptor_index = 0;
  uint16_t attrs_count = 0;
  vector<AttrInfo> attrs;
};

struct MethodInfo : FieldInfo
{
};

struct ClassFile
{
  uint16_t minor_version = 0;
  uint16_t major_version = 0;
  uint16_t const_pool_count = 0;
  vector<ConstPoolInfo> const_pool; // 1 ~ const_pool_count-1
  uint16_t access_flags = 0;
  uint16_t this_class = 0;
  uint16_t super_class = 0;
  vector<uint16_t> interfaces;
  vector<FieldInfo> fields;
  vector<MethodInfo> methods;
  uint16_t attrs_count = 0;
  vector<AttrInfo> attrs;
};

