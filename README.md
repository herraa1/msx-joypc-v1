# PC gameport joystick controller adapter for MSX (msx-joypc) v1

[<img src="images/msx-joypc-logo-same-color.png" width="256"/>](images/msx-joypc-logo-same-color.png)

Connect [PC gameport joysticks](https://en.wikipedia.org/wiki/Game_port) to [MSX computers](https://www.msx.org/wiki/).

> [!NOTE]
>
> No build instructions are yet provided, but if you are brave enough go for the [recommended build](#recommended-build).
>

## Introduction

The msx-joypc v1 is an adapter that allows connecting PC gameport joysticks to [MSX general purpose I/O ports](https://www.msx.org/wiki/General_Purpose_port).

The main features of the msx-joypc v1 adapter are:
* small footprint
* made of widely available electronic components
* behaves as a cord extension between the MSX computer and the PC gameport joystick
* uses a female standard DE9 connector on the adapter's MSX joystick side
* uses a [DA-15](https://en.wikipedia.org/wiki/D-subminiature) socket on the adapter's PC gameport joystick side
* no need for external power supply, the adapter draws current from the MSX port
* low power consumption (<20mA)
* serial debug provides information about the operation of the adapter

## [Hardware](hardware/kicad/)

The msx-joypc v1 adapter uses an [Atmega328p](https://en.wikipedia.org/wiki/ATmega328) to convert the [PC gameport joystick signalling](https://www.epanorama.net/documents/joystick/pc_joystick.html) to the [MSX joystick standard signalling](https://www.msx.org/wiki/Joystick_control).

A two-sided printed circuit board (PCB) is used to put together all components:
* An Atmega328p as the main MCU, running at 5V 16MHz
* Two 74LS03 quad 2-input NAND gates with open collectors to completely mimic the standard MSX joystick behavior
* A PTC fuse to minimize damage to the MSX computer in case something goes wrong with the board
* Several additional required components (crystal, diode, leds, resistors, ceramic capacitors and a tantalum capacitor)
* An angled DA-15 socket is used to directly connect the PC gameport joystick
* A PH2.0 connector is used to connect the MSX cable extension
* A 2.54 pitch debug header is added for serial debug and programming
* An ICSP header is provided for burning a bootloader or to flash the chip using a hardware programmer
* A DIP switch to configure the adapter for compatibility with different PC joysticks

[<img src="images/msx-joypc-v1-build2-front-render.png" width="512"/>](images/msx-joypc-v1-build2-front-render.png)
[<img src="images/msx-joypc-v1-build2-back-render.png" width="512"/>](images/msx-joypc-v1-build2-back-render.png)

Connection to the MSX general purpose I/O port is implemented using a DE9 joystick extension cable with a female DE9 connector on one side and a loose end on the other side.
The MSX joystick extension cable loose end is wired according to the following pinout mapping.

| [<img src="images/msx_joystick-white-bg.png" height="100"/>](images/msx_joystick-white-bg.png) |
|:--|
| MSX joystick connector pinout, from controller plug side |

| MSX side pin | Cable color (may vary) | Signal |
| ------------ | ---------------------- | ------ |
| 5            | Brown                  | +5v    |
| 4            | Orange                 | RIGHT  |
| 3            | Grey                   | LEFT   |
| 2            | Black                  | DOWN   |
| 1            | Red                    | UP     |
| 6            | Green                  | TRIGA  |
| 7            | White                  | TRIGB  |
| 8            | Blue                   | OUT    |
| 9            | Yellow                 | GND    |

The msx-joypc v1 is fully compatible with MSX joysticks and honors the pin8 (OUT) signal.

The adapter uses open collector outputs (using 74LS03 Quad 2-Input NAND gates with open collector outputs) which makes the adapter safer [^2] than the
standard MSX joystick schematic depicted in the MSX Technical Data Book, as it avoids a series of undesired conditions that can lead to bus contention/short circuits.

Power is drawn from the +5V signal of the MSX general purpose I/O port, which is capable of delivering up to 50mA [^1].
The msx-joypc v1 draws below 20mA from the port when a controller is connected, so it is on the safe side. Anyway, the msx-joypc adapter uses a Positive Temperature Coeficient (PTC) resettable fuse of 50mA (F1) to limit current in case something goes wrong.

Also on the power side, a [1N5819 Schottky diode](https://www.diodes.com/assets/Datasheets/1N5819HW.pdf) (D1) is used to avoid leaking current from the msx-joypc adapter to the MSX in case power is applied to one of the VCC pins available on the ICSP or Debug headers while the adapter is plugged into an MSX.

Connection to the PC gameport joystick is done via a PC gameport joystick socket.

| [<img src="images/gameport-wiring.jpg" height="256"/>](images/gameport-wiring.jpg) |
|:--|
| Gameport connector pinout, from PC gameport joystick side |

| Gameport pin   | Signal       | Comment                                                                   |
| -------------- | -------------| ------------------------------------------------------------------------- |
| 1              | +5V          | +5V power                                                                 |
| 2              | B1           | Button1                                                                   |
| 3              | X1           | Joystick 1 - X axis                                                       |
| 4              | GND          | ground                                                                    |
| 5              | GND          | ground                                                                    |
| 6              | Y1           | Joystick 1 - Y axis                                                       |
| 7              | B2           | Button2                                                                   |
| 8              | +5V          | +5V power                                                                 |
| 9              | +5V          | +5V power                                                                 |
| 10             | B3           | Button3                                                                   |
| 11             | X2           | Joystick 2 - X axis                                                       |
| 12             | GND/MIDI_OUT | ground / MIDI out                                                         |
| 13             | Y2           | Joystick 2 - Y axis                                                       |
| 14             | B4           | Button4                                                                   |
| 15             | +5V/MIDI_IN  | +5V power / MIDI_IN                                                       |

The PC gameport joystick uses 5V for power and logic.

Power for the PC gameport joystick is provided by the MSX +5V rail.

### Recommended Build

Please, use [msx-joypc-v1 Build2](#build2) for making new boards.

### [Build2](hardware/kicad/msx-joypc-v1-build2)

[Bill Of Materials (BoM)](https://html-preview.github.io/?url=https://raw.githubusercontent.com/herraa1/msx-joypc-v1/main/hardware/kicad/msx-joypc-v1-build2/bom/ibom.html)

This build fixes the following errata:
- make the ICSP header work again
- add (optional) RN1 and RN2 4k7 pull-downs to joystick signals to avoid spurious button joystick actions while powering up

This board has been successfully built and works fine.

[<img src="images/msx-joypc-v1-build2-bare-pcb-front.png" width="320"/>](images/msx-joypc-v1-build2-bare-pcb-front.png)
[<img src="images/msx-joypc-v1-build2-bare-pcb-back.png" width="320"/>](images/msx-joypc-v1-build2-bare-pcb-back.png)

[<img src="images/msx-joypc-v1-build2-assembled-pcb-front.png" width="320"/>](images/msx-joypc-v1-build2-assembled-pcb-front.png)
[<img src="images/msx-joypc-v1-build2-assembled-pcb-back.png" width="320"/>](images/msx-joypc-v1-build2-assembled-pcb-back.png)

### Build1

> [!WARNING]
> This build is broken. I messed up the ICSP header after a last-minute pin reordering. Meh.
> It can be fixed by cutting two traces, exposing two traces and using a pair of jumper wires to bridge the necessary points.
>
> Do NOT build. Use the [recommended build](#recommended-build) instead.
>

[<img src="images/msx-joypc-v1-build1-front-fix.png" width="256"/>](images/msx-joypc-v1-build1-front-fix.png)
[<img src="images/msx-joypc-v1-build1-back-fix.png" width="256"/>](images/msx-joypc-v1-build1-back-fix.png)

## [Firmware](firmware/msx-joypc-v1/)

The msx-joypc v1 adapter firmware uses a slightly modified version of [Necroware's GamePort adapter firmware](https://github.com/necroware/gameport-adapter) to read the PC gameport joystick status.

The following elements are used as inputs:
* joystick X and Y axis as direction arrows
* button 1, as Trigger 1
* button 2, as Trigger 2

Those elements' status are processed by the msx-joypc firmware and transformed into MSX general purpose I/O port's signals on the fly.

## [Enclosure](enclosure/)

### Acrylic

A simple acrylic enclosure design for the project is provided to protect the electronic components and provide strain relief for the extension cord.

The enclosure uses a 3mm acrylic sheet.

[<img src="images/msx-joypc-v1-build2-with-enclosure-general-view.png" width="400"/>](images/msx-joypc-v1-build2-with-enclosure-general-view.png)

## Configuration Switches

[<img src="images/msx-joypc-v1-build2-switches.png" width="512"/>](images/msx-joypc-v1-build2-switches.png)

Joystick Model               | Buttons | Axes  |  Hat | SW1-4 | Protocol   | Comments
-----------------------------|---------|-------|------|-------|------------|------------------------------------
Generic Analog               | 2       | 2     | 0    | 0000  | Analogue   |
Generic Analog               | 4       | 2     | 0    | 1000  | Analogue   |
Generic Analog               | 4       | 3     | 0    | 0100  | Analogue   | 3rd Axis is throttle
Generic Analog               | 4       | 4     | 0    | 1100  | Analogue   |
CH FlightStick               | 4       | 4     | 1    | 0010  | Analogue   |
CH F16 Combat Stick          | 10      | 3     | 1    | 0110  | Analogue   |
ThrustMaster                 | 4       | 3     | 1    | 1010  | Analogue   |
Sidewinder GamePad           | 10      | 2     | 0    | 1110  | Sidewinder |
Sidewinder 3D Pro            | 8       | 4     | 1    | 1110  | Sidewinder |
Sidewinder 3D Pro Plus       | 9       | 4     | 1    | 1110  | Sidewinder | First version of Precision Pro
Sidewinder Precision Pro     | 9       | 4     | 1    | 1110  | Sidewinder |
Sidewinder FFB Pro           | 9       | 4     | 1    | 1110  | Sidewinder | FFB not yet implemented
Sidewinder FFB Wheel         | 8       | 3     | 0    | 1110  | Sidewinder | FFB not yet implemented
Gravis GamePad Pro           | 10      | 2     | 0    | 0001  | GrIP       |
Logitech WingMan Extreme     | 6       | 3     | 1    | 1001  | ADI        |
Logitech CyberMan 2          | 8       | 6     | 0    | 1001  | ADI        |
Logitech WingMan Interceptor | 9       | 3     | 3    | 1001  | ADI        | 2 hats are mapped as 4 axes
Logitech ThunderPad Digital  | 8       | 2     | 0    | 1001  | ADI        | Directional buttons mapped as 2 axes
Logitech WingMan Gamepad     | 11      | 2     | 0    | 1001  | ADI        | Directional buttons mapped as 2 axes
Logitech WingMan Light       | 2       | 2     | 0    | 0000  | Analogue   |

For an up to date information see [Which joysticks does this adapter support](https://github.com/necroware/gameport-adapter?tab=readme-ov-file#which-joysticks-does-this-adapter-support) on [Necroware's GamePort adapter project](https://github.com/necroware/gameport-adapter).

## LED indicators

[<img src="images/msx-joypc-v1-build2-leds.png" width="512"/>](images/msx-joypc-v1-build2-leds.png)

| **LED**   | **State**      | **Indication** |
|-----------|----------------|---------------|
| _POWER_   | Off            | board is not receiving 5V power |
| _POWER_   | Solid Red      | board is receiving 5V power     |
| _JoyPort_ | Off            | no PC joystick connected or recognized |
| _JoyPort_ | Solid Green    | PC joystick connected and ready to use |

## References

MSX general purpose I/O port
* https://www.msx.org/wiki/General_Purpose_port

Atmega328p pinout
- https://camo.githubusercontent.com/06bc0a6bb71729956af107f56c3e57bd065d9fb924bcc7f6bb8fbb487a1a5e55/68747470733a2f2f692e696d6775722e636f6d2f6e6177657145362e6a7067

Joystick PC Gameport pinout
* https://allpinouts.org/pinouts/connectors/input_device/joystick-pc-gameport/

PC joystick interface
* https://www.epanorama.net/documents/joystick/pc_joystick.html

Necroware's GamePort adapter project
* https://github.com/necroware/gameport-adapter

Using a PC Joystick with the Arduino
* http://www.built-to-spec.com/blog/2009/09/10/using-a-pc-joystick-with-the-arduino/

Arduino gameport interface
* https://build-its.blogspot.com/2012/01/arduino-game-port-interface.html

DIY gameport to USB adapter
* https://littlebigtech.net/posts/diy-gameport-to-usb-adapter/

## Image Sources

* https://www.oshwa.org/open-source-hardware-logo/
* https://en.wikipedia.org/wiki/File:Numbered_DE9_Diagram.svg

[^1]: https://www.msx.org/wiki/General_Purpose_port
[^2]: https://www.msx.org/wiki/Joystick/joypad_controller (see "Undesired Conditions")

