#pragma once

#include <cstdint>

namespace cnfx {
namespace can {

uint8_t read_node_id(uint32_t sel0, uint32_t sel1, uint32_t sel2);

}  // namespace can
}  // namespace cnfx
