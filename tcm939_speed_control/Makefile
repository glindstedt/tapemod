MCU=attiny13
AVRDUDEMCU=t13
CC=/usr/bin/avr-gcc
CFLAGS=-g -Os -Wall -mcall-prologues -mmcu=$(MCU)
OBJ2HEX=/usr/bin/avr-objcopy
AVRDUDE=/usr/bin/avrdude
TARGET=main
BAUDRATE=8000
SRC=main.c lookups.h

all: main.hex

main.hex: $(SRC)
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET) $(TARGET).hex
	rm -f $(TARGET)


install: main.hex
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -P /dev/spidev0.0 -c linuxspi -b $(BAUDRATE) -U flash:w:$(TARGET).hex

fuse:
	# DON'T KNOW WHAT THIS DOES YET
	#sudo $(AVRDUDE) -p $(AVRDUDEMCU) -P /dev/spidev0.0 -c linuxspi -b $(BAUDRATE) -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

clean:
	rm -f *.hex *.obj *.o
