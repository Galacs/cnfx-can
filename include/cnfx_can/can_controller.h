#pragma once

#include <Arduino.h>

#include <cstring>

#include "can_struct.h"

#if defined(ARDUINO_ARCH_STM32)
#include "STM32_CAN.h"
#elif defined(ARDUINO_ARCH_ESP32)
#include "driver/twai.h"
#else
#error "cnfx-can supports only STM32 and ESP32"
#endif

namespace cnfx {
namespace can {

typedef void (*can_handler_t)(const can_frame_t& frame);

template <typename Msg>
struct TypedHandler {
  typedef void (*Handler)(const Msg&);

  static Handler& handler() {
    static Handler fn = nullptr;
    return fn;
  }

  static void call(const can_frame_t& frame) {
    if (frame.len != static_cast<uint8_t>(sizeof(Msg))) {
      return;
    }
    Msg msg;
    memcpy(&msg, frame.data, sizeof(Msg));
    if (handler()) {
      handler()(msg);
    }
  }
};

class CanController {
 public:
#if defined(ARDUINO_ARCH_ESP32)
  bool begin(gpio_num_t rx_pin, gpio_num_t tx_pin);
#elif defined(ARDUINO_ARCH_STM32)
  bool begin();
#endif

  bool register_handler(uint32_t id, can_handler_t handler);

  template <typename Msg>
  bool register_handler(void (*handler)(const Msg&)) {
    TypedHandler<Msg>::handler() = handler;
    return register_handler(Msg::ID, &TypedHandler<Msg>::call);
  }

  bool send(uint32_t id, const void* data, uint8_t len);

  template <typename Msg>
  bool send(const Msg& msg) {
    return send(Msg::ID, &msg, static_cast<uint8_t>(sizeof(Msg)));
  }

  void poll();

 private:
  struct HandlerEntry {
    uint32_t id;
    can_handler_t handler;
  };

  static constexpr uint8_t MAX_HANDLERS = 8;
  HandlerEntry handlers_[MAX_HANDLERS];
  uint8_t handler_count_ = 0;

  bool send_frame(const can_frame_t& frame);
  bool receive_frame(can_frame_t* frame);

#if defined(ARDUINO_ARCH_STM32)
  STM32_CAN m_can{CAN1, DEF};
#endif
};

}  // namespace can
}  // namespace cnfx
