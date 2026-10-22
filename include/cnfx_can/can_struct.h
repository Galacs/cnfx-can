#pragma once

#include <cstdint>
#include <type_traits>

namespace cnfx {
namespace can {

struct can_frame_t {
  uint32_t id = 0;
  uint8_t data[8] = {0};
  uint8_t len = 0;
};

constexpr uint16_t make_id(uint8_t library, uint8_t message) {
  return (uint16_t(library) << 6) | (message & 0x3F);
}

}  // namespace can
}  // namespace cnfx

#define CAN_STRUCT(struct_name, can_id, ...)                                 \
  struct __attribute__((packed)) struct_name {                               \
    static constexpr uint32_t ID = (can_id);                                 \
    __VA_ARGS__                                                              \
  };                                                                         \
  static_assert((can_id) <= 0x7FFu, #struct_name " CAN id exceeds 11 bits"); \
  static_assert(sizeof(struct_name) <= 8u,                                   \
                #struct_name " exceeds CAN frame size (8 bytes)");           \
  static_assert(std::is_trivially_copyable<struct_name>::value,              \
                #struct_name " must be trivially copyable")
