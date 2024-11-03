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

#pragma once

#include "Buffer.h"
#include "Joystick.h"
#include "Utilities.h"
#include <Arduino.h>

#include "DigitalPin.h"
#include <avr/wdt.h>

void __force_system_reset(void) {
	MCUSR = 0;
	wdt_enable(WDTO_250MS);
	while(1)  ;  // force WDT to reset system
}

class MsxJoystick {
public:
  bool init(Joystick *joystick) {
    
    if (!joystick || !joystick->init()) {
      return false;
    }

    m_joystick = joystick;
    m_state = State::BOOT_CONNECTED_CONNECTED;
    m_led.setHigh();

    const auto &desc = m_joystick->getDescription();
    log("Configured device: %s (%d axes, %d buttons)", desc.name, desc.numAxes, desc.numButtons);

    return true;
  }

  bool update() {
    if (!m_joystick || !m_joystick->update()) {
      return false;
    }

    const auto &state = m_joystick->getState();
    const auto &desc = m_joystick->getDescription();

#ifdef DIAG
    static auto last_state = m_joystick->getState();

    for (auto i = 0u; i < desc.numAxes; i++) {
        log_nolf("axes[%d] = %d,", i, state.axes[i]);
    }

    if (desc.hasHat) {
      if (last_state.hat != state.hat) {
        log_nolf("hat = %d,", state.hat);
      }
    }

    if (desc.numButtons) {
      if (last_state.buttons != state.buttons) {
        log_nolf("buttons = %d", state.buttons);
      }
    }
    log_nolf("\n");

    last_state = state;
#endif

    int16_t lx, ly;

    ly = (1 < desc.numAxes)?state.axes[1]:s_axis_threshold_min;
    lx = (0 < desc.numAxes)?state.axes[0]:s_axis_threshold_min;

    if (lx == 1023 && ly == 1023) {
        switch(m_state) {
            case State::BOOT_CONNECTED_CONNECTED:
                m_state = State::BOOT_CONNECTED_UNCONNECTED_MAYBE;
                m_last_state_millis = millis();
                break;
            case State::BOOT_CONNECTED_UNCONNECTED_MAYBE:
                unsigned long current_millis = millis();
                if (current_millis - m_last_state_millis >= s_joystick_dead_time) {
                    // adapter started with joystick connected, but joystick was disconnected
                    m_state = State::BOOT_CONNECTED_UNCONNECTED_CONFIRMED;
                    m_last_state_millis = millis();
                    log("Adapter booted with joystick connected, but now joystick is disconnected");
                    m_led.setLow();
                }
                break;
        }
    } else if (lx == 511 && ly == 511) {
        switch(m_state) {
            case State::BOOT_CONNECTED_CONNECTED:
                m_state = State::BOOT_UNCONNECTED_UNCONNECTED_MAYBE;
                m_last_state_millis = millis();
                break;
            case State::BOOT_UNCONNECTED_UNCONNECTED_MAYBE:
                unsigned long current_millis = millis();
                if (current_millis - m_last_state_millis >= s_joystick_dead_time) {
                    // adapter started with joystick not connected, joystick still disconnected
                    m_state = State::BOOT_UNCONNECTED_UNCONNECTED_CONFIRMED;
                    m_last_state_millis = millis();
                    log("Adapter booted with joystick disconnected");
                    m_led.setLow();
                    __update_msx_signals(0);
                }
                break;
        }
    } else {
        switch(m_state) {
            case State::BOOT_CONNECTED_UNCONNECTED_CONFIRMED:
                // fallthrough
            case State::BOOT_UNCONNECTED_UNCONNECTED_CONFIRMED:
                log("Joystick reconnected, restarting adapter!");
                delay(5);
                __force_system_reset();
                break;
        }
        m_state = State::BOOT_CONNECTED_CONNECTED;
    }

    uint8_t joystick_signals = 0x00;

    if (m_state == State::BOOT_CONNECTED_CONNECTED) {
        if ((state.buttons & 0x01) || (state.buttons & 0x04))
            joystick_signals |= (1 << MsxPortPins::MSX_JOYSTICK_TRIGGER1);

        if ((state.buttons & 0x02) || (state.buttons & 0x08))
            joystick_signals |= (1 << MsxPortPins::MSX_JOYSTICK_TRIGGER2);

        if ((ly > s_axis_threshold_max) && !(joystick_signals & (1 << MsxPortPins::MSX_JOYSTICK_UP)))
            joystick_signals |= (1 << MsxPortPins::MSX_JOYSTICK_DOWN);

        /* before activating UP check that DOWN is not activated */
        if ((ly < s_axis_threshold_min) && !(joystick_signals & (1 << MsxPortPins::MSX_JOYSTICK_DOWN)))
            joystick_signals |= (1 << MsxPortPins::MSX_JOYSTICK_UP);

        /* before activating LEFT check that RIGHT is not activated */
        if ((lx < s_axis_threshold_min) && !(joystick_signals & (1 << MsxPortPins::MSX_JOYSTICK_RIGHT)))
            joystick_signals |= (1 << MsxPortPins::MSX_JOYSTICK_LEFT);

        /* before activating RIGHT check that LEFT is not activated */
        if ((lx > s_axis_threshold_max) && !(joystick_signals & (1 << MsxPortPins::MSX_JOYSTICK_LEFT)))
            joystick_signals |= (1 << MsxPortPins::MSX_JOYSTICK_RIGHT);
    }

    __update_msx_signals(joystick_signals);

    return true;
  }

private:
  static constexpr unsigned long s_joystick_dead_time = 2UL * 1000UL; // 2 seconds
  static constexpr auto s_axis_threshold = (1023 - 0) / 3;
  static constexpr auto s_axis_threshold_min = 0 + s_axis_threshold;
  static constexpr auto s_axis_threshold_max = 1023 - s_axis_threshold;

  Joystick *m_joystick{};
  DigitalOutput<13> m_led;

  enum class State {
    BOOT_CONNECTED_CONNECTED,
    BOOT_CONNECTED_UNCONNECTED_MAYBE,
    BOOT_CONNECTED_UNCONNECTED_CONFIRMED,
    BOOT_UNCONNECTED_UNCONNECTED_MAYBE,
    BOOT_UNCONNECTED_UNCONNECTED_CONFIRMED
  };

  unsigned long m_last_state_millis;
  State m_state;

#define PORT_MSX_JOYSTICK PORTD
#define DDR_MSX_JOYSTICK  DDRD

  enum MsxPortPins {
    /* MSX general purpose signals as mapped to Atmega328p PORTD bits */
    MSX_JOYSTICK_UP = 7,       /* PD7, MSX joystick pin1 */
    MSX_JOYSTICK_DOWN = 6,     /* PD6, MSX joystick pin2 */
    MSX_JOYSTICK_LEFT = 5,     /* PD5, MSX joystick pin3 */
    MSX_JOYSTICK_RIGHT = 4,    /* PD4, MSX joystick pin4 */
    MSX_JOYSTICK_TRIGGER1 = 3, /* PD3, MSX joystick pin6 */
    MSX_JOYSTICK_TRIGGER2 = 2, /* PD2, MSX joystick pin7 */
  };

  static constexpr bool direct_mode = false; // direct_mode uses open collector outputs (use only in test prototype without 74LS03)

  static inline void __update_msx_signals(uint8_t signals)
  {
    /* write all signal states at once to MSX side */
    if (direct_mode) {
        PORT_MSX_JOYSTICK = ~signals;
        DDR_MSX_JOYSTICK = signals;
    } else {
        PORT_MSX_JOYSTICK = signals;
        DDR_MSX_JOYSTICK = ~0;
    }
  }

};
