Speed Control for TCM939
========================

Code is written for the ATTiny13 and is based on the code from Syntherjack:
[https://syntherjack.net/cassette-recorder-with-cv-speed-controller/](https://syntherjack.net/cassette-recorder-with-cv-speed-controller/)

# Compiling

[http://www.nongnu.org/avr-libc/user-manual/index.html](http://www.nongnu.org/avr-libc/user-manual/index.html)

```bash
# Install dependencies
sudo apt install gcc-avr binutils-avr avr-libc

make
```

# Flashing

[https://en.wikipedia.org/wiki/Serial_Peripheral_Interface](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface)

## Wiring

[https://pinout.xyz](https://pinout.xyz)

```bash
sudo apt install python3-gpiozero && pinout
```

## Configure the linuxspi programmer

```
# /etc/avrdude.conf
programmer
  id = "linuxspi";
  desc = "Use Linux SPI device in /dev/spidev*";
  type = "linuxspi";
  reset = 22;
  # reset = 25;
  baudrate=400000;
;
```

## Check the connections

```bash
# Baud rate of 8000 seems fairly reliable try something lower if you have issues.
sudo avrdude -p attiny13 -P /dev/spidev0.0 -c linuxspi -b 8000
```

# Notes

## Calculating the PWM frequency

The ATTiny13 uses an internal 9.6Mhz crystal oscillator as its default clock
source, and comes preprogrammed with the CKDIV8 fuse which divides the
oscillator source by 8. This can also be set through code using the system
clock prescaler.

The Timer/Counter0 module has its own prescaler configuration which can further
divide the clock source.

Since this is an 8-bit counter this means counting from BOTTOM to TOP takes 256
clock cycles, and to get a full PWM period we would need to count (up or down)
4 times. This means that getting a full PWM period takes 4 * 256 = 1024 clock
cycles.
