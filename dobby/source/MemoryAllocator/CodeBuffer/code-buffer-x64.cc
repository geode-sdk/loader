#include "platform_macro.h"
#if defined(TARGET_ARCH_X64)

// alk
#include <iomanip>
#include <iostream>


#include "MemoryAllocator/CodeBuffer/code-buffer-x64.h"

void CodeBuffer::Emit32(int32_t data) {
	std::cout << std::hex << "Emit32: " << +data << std::endl;
	std::cout << "cursor: " << (size_t)buffer_cursor << std::endl;
  ensureCapacity(getSize() + sizeof(int32_t));
  *reinterpret_cast<int32_t *>(getCursor()) = data;
  buffer_cursor += sizeof(int32_t);
  return;
}

void CodeBuffer::Emit64(int64_t data) {
	std::cout << std::hex << "Emit64: " << +data << std::endl;
	std::cout << "cursor: " << (size_t)buffer_cursor << std::endl;
  ensureCapacity(getSize() + sizeof(int64_t));
  *reinterpret_cast<int64_t *>(getCursor()) = data;
  buffer_cursor += sizeof(int64_t);
  return;
}

void CodeBuffer::FixBindLabel(int offset, int32_t disp) {
  *reinterpret_cast<uint32_t *>(buffer + offset) = disp;
  return;
}

#endif
