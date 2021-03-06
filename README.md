Fork of Contiki OS from Colibri (https://github.com/persuader72/contiki) for the Olimexino5510 platform:
https://www.olimex.com/Products/Duino/MSP430/OLIMEXINO-5510/open-source-hardware

 - MSP430F5510 MCU with USB/printf support via the msp430 USB API, ported to work with msp430-gcc (https://github.com/jlhonora/usb-mspgcc/tree/master/src)

Changes:
 - Removed external flash code (based on AT25F512B)
 - Removed MRF49XA radio code.
 - Changed Makefiles:
  - remove -fstack-check (which seemed to stop the device from enumerating as a USB device)
  - Compile with -Os to squeeze program size.
  - By default programs using python msp430-bsl library (https://github.com/cetic/python-msp430-tools)
  
Planned:
 - Add support for the MOD-MRF24J40 IEEE 802.15.4 radio.
 
To compile:
```
make hello-world.ihex
 ```

To upload (make sure the python bsl library is installed).
```
make hello-world.upload
```

To login to serial port:
```
make login
```

Or specify the port:
```
make [port].login
```
