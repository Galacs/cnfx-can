#include "cnfx_can/can_controller.h"

#include <cstring>

namespace cnfx {
namespace can {

bool CanController::register_handler(uint32_t id, can_handler_t handler) {
  for (uint8_t i = 0; i < handler_count_; ++i) {
    if (handlers_[i].id == id) {
      handlers_[i].handler = handler;
      return true;
    }
  }

  if (handler_count_ >= MAX_HANDLERS) {
    return false;
  }

  handlers_[handler_count_].id = id;
  handlers_[handler_count_].handler = handler;
  ++handler_count_;
  return true;
}

bool CanController::send(uint32_t id, const void *data, uint8_t len) {
  if (len > 8) {
    return false;
  }

  can_frame_t frame = {};
  frame.id = id;
  frame.len = len;
  memcpy(frame.data, data, len);
  return send_frame(frame);
}

void CanController::poll() {
  can_frame_t frame;

  while (receive_frame(&frame)) {
    for (uint8_t i = 0; i < handler_count_; ++i) {
      if (handlers_[i].id == frame.id) {
        handlers_[i].handler(frame);
      }
    }
  }
}

#if defined(ARDUINO_ARCH_STM32)

bool CanController::begin() {
  m_can.begin();
  m_can.setBaudRate(500000);
  m_can.setFilter(0, 0, 0);
  return true;
}

bool CanController::send_frame(const can_frame_t &frame) {
  CAN_message_t msg = {};
  msg.id = frame.id;
  msg.len = frame.len;
  msg.flags.remote = false;
  msg.flags.extended = false;
  memcpy(msg.buf, frame.data, frame.len);
  return m_can.write(msg);
}

bool CanController::receive_frame(can_frame_t *frame) {
  CAN_message_t msg = {};

  if (!m_can.read(msg)) {
    return false;
  }
  if (msg.flags.remote || msg.len > 8) {
    return false;
  }

  frame->id = msg.id;
  frame->len = msg.len;
  memcpy(frame->data, msg.buf, msg.len);
  return true;
}

#elif defined(ARDUINO_ARCH_ESP32)

bool CanController::begin(gpio_num_t rx_pin, gpio_num_t tx_pin) {
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(tx_pin, rx_pin, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    return false;
  }
  if (twai_start() != ESP_OK) {
    twai_driver_uninstall();
    return false;
  }
  return true;
}

bool CanController::send_frame(const can_frame_t &frame) {
  twai_message_t msg = {};
  msg.identifier = frame.id;
  msg.extd = false;
  msg.rtr = false;
  msg.data_length_code = frame.len;
  memcpy(msg.data, frame.data, frame.len);
  return twai_transmit(&msg, pdMS_TO_TICKS(0)) == ESP_OK;
}

bool CanController::receive_frame(can_frame_t *frame) {
  twai_message_t msg = {};

  if (twai_receive(&msg, 0) != ESP_OK) {
    return false;
  }
  if (msg.rtr || msg.data_length_code > 8) {
    return false;
  }

  frame->id = msg.identifier;
  frame->len = msg.data_length_code;
  memcpy(frame->data, msg.data, msg.data_length_code);
  return true;
}

#endif

} // namespace can
} // namespace cnfx
