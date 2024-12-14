/*
 * msx-joypc v1, a PC gameport to MSX joystick adapter
 * Copyright (C) 2024 Albert Herranz
 *
 * This code is released under GPL V2.0
 *
 * Valid-License-Identifier: GPL-2.0-only
 * SPDX-URL: https://spdx.org/licenses/GPL-2.0-only.html
 *
 */
 /*

Built using Arduino 2.2.1
Boards:
- Minicore 2.2.2
  - Board: Atmega328
  - BOD 1.8V
  - Bootloader: Yes (UART0)
  - Clock: External 16MHz
  - EEPROM: Retained
  - Compiler LTO: Disabled
  - Variant: 328P/328PA

  */
//
// Based on Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "DigitalPin.h"
#include "MsxJoystick.h"

#include "CHFlightstickPro.h"
#include "CHF16CombatStick.h"
#include "GenericJoystick.h"
#include "GrIP.h"
#include "Logitech.h"
#include "Sidewinder.h"
#include "ThrustMaster.h"

#include <avr/wdt.h>

static Joystick *createJoystick() {

  const auto sw1 = DigitalInput<12, true>{};
  const auto sw2 = DigitalInput<A1, true>{};
  const auto sw3 = DigitalInput<A2, true>{};
  const auto sw4 = DigitalInput<A3, true>{};

  // Give some time to setup the input
  delay(1);

  const auto sw = !sw4 << 3 | !sw3 << 2 | !sw2 << 1 | !sw1;
  log("Configuration switches = 0x%0x", sw);

  switch (sw) {
    case 0b0001:
      return new GenericJoystick<2,4>;
    case 0b0010:
      return new GenericJoystick<3,4>;
    case 0b0011:
      return new GenericJoystick<4,4>;
    case 0b0100:
      return new CHFlightstickPro;
    case 0b0101:
      return new ThrustMaster;
    case 0b0110:
      return new CHF16CombatStick;
    case 0b0111:
      return new Sidewinder;
    case 0b1000:
      return new GrIP;
    case 0b1001:
      return new Logitech;
    default:
      return new GenericJoystick<2,2>;
  }
}

void setup() {
    MCUSR = 0;
    wdt_disable();

    // DEBUG information: Debugging is turned off by default
    // Comment the "NDEBUG" line in "Utilities.h" to enable logging to the serial monitor
    initLog();
    log("msx-joypc-v1 20241110_1");
}

void loop() {
    static auto msxJoystick = [] {
        MsxJoystick msxJoystick;
        msxJoystick.init(createJoystick());
        return msxJoystick;
    }();

    msxJoystick.update();
    delay(2);
}
