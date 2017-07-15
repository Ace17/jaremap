#pragma once

#include <stdint.h>

struct OutputStream
{
  virtual void write(const uint8_t* dst, size_t len) = 0;
};

