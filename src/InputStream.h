#pragma once

#include <stdint.h>

struct InputStream
{
  virtual void read(uint8_t* dst, size_t len) = 0;
};

