# cnfx-can

Small CAN transport library for CNFX robots. It contains **no application
message definitions**: each module repository owns its `CAN_STRUCT` messages
and its own node/channel semantics.

The controller is a simple `(CAN id -> function pointer)` table. A module
registers the IDs it handles; the core never needs to know the messages.

`make_id(library, message)` builds the ID. The `CAN_STRUCT` macro checks that the
result fits 11 bits, that the struct is at most 8 bytes and that it is trivially
copyable.

## Defining messages

Messages are defined in module repositories, not in this repository. A module
header looks like this:

```cpp
#include <cnfx_can/can_struct.h>

CAN_STRUCT(t_my_message, cnfx::can::make_id(2, 0),
  uint8_t node;
  uint16_t value;
);
```

`CAN_STRUCT` is the only message-related code in this repository.

## Controller API

```cpp
namespace cnfx { namespace can {

class CanController {
 public:
#if defined(ARDUINO_ARCH_ESP32)
  bool begin(gpio_num_t rx_pin, gpio_num_t tx_pin);
#elif defined(ARDUINO_ARCH_STM32)
  bool begin();
#endif

  bool register_handler(uint32_t id, can_handler_t handler);

  template <typename Msg>
  bool register_handler(void (*handler)(const Msg&));

  bool send(uint32_t id, const void* data, uint8_t len);

  template <typename Msg>
  bool send(const Msg& msg);

  void poll();
};
}}
```

- `register_handler(id, fn)` installs or replaces a raw frame handler.
  Up to 8 handlers are supported.
- `register_handler<Msg>(fn)` is the typed version: the core checks the frame
  length, copies it into `Msg` and calls `fn(const Msg&)`. The module callback
  never sees a frame or does `memcpy`.
- `poll()` drains the hardware receive queue and calls the matching handler.
  Unknown IDs, RTR frames and frames with more than 8 bytes are ignored.
- `send(msg)` copies the struct into a frame using `Msg::ID`.
- `begin()` returns `false` if the peripheral cannot be started. It does not
  print anything.

The length check and `memcpy` live once in `TypedHandler<Msg>` in the CAN core;
modules only register typed callbacks.

## Node ID

`read_node_id(sel0, sel1, sel2)` reads three `INPUT_PULLUP` pins. A jumper to
GND sets a bit, so no jumpers means node 0. Define `CNFX_NODE_ID` to override
the pins (useful on boards without SEL jumpers).

## Platforms

- **STM32**: `STM32_CAN` on CAN1 (PA11/PA12), 500 kbit/s. The project needs
  `build_flags = -DHAL_CAN_MODULE_ENABLED` and `pazi88/STM32_CAN@^1.2.2`.
- **ESP32**: TWAI, `begin(rx_pin, tx_pin)`, 500 kbit/s.

Only classic CAN is supported: 8 data bytes, no CAN-FD, no 29-bit API.

## Import

```ini
lib_deps =
	https://github.com/Galacs/cnfx-can.git
```

For local development in this workspace the dc-drv projects use
`symlink://../../../cnfx-can`.

## Adding a module

1. Pick a library ID (or use the shared `MOTION = 1` block).
2. In the new module repository, define `CAN_STRUCT` messages.
3. Register handlers in the module firmware with `register_handler`.
4. Send with `can.send(msg)`.
