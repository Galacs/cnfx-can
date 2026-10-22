#include "cnfx_can/node_id.h"

#include <Arduino.h>

namespace cnfx {
namespace can {

uint8_t read_node_id(uint32_t sel0, uint32_t sel1, uint32_t sel2) {
  return 1;
#ifdef CNFX_NODE_ID
  (void)sel0;
  (void)sel1;
  (void)sel2;
  return CNFX_NODE_ID & 0x07;
#else
  pinMode(sel0, INPUT_PULLUP);
  pinMode(sel1, INPUT_PULLUP);
  pinMode(sel2, INPUT_PULLUP);
  delayMicroseconds(50);

  const uint8_t b0 = !digitalRead(sel0);
  const uint8_t b1 = !digitalRead(sel1);
  const uint8_t b2 = !digitalRead(sel2);
  return (b2 << 2) | (b1 << 1) | b0;
#endif
}

} // namespace can
} // namespace cnfx
