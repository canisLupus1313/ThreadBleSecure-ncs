Fork of https://github.com/arnulfrupp/ThreadBleSecure

Environment setup:
==================
https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started/installing.html

Init repo:
==========

```
west init -m https://github.com/ --mr main my-workspace
```

Build:
======

```
cd my-workspace
west build -b <board> ThreadBleSecure-ncs
```

Flashing board:
===============
- boards containing jlink eg. Nordic nRF52840dk

    ```
    west flash
    ```

- boadrs with usb flasher eg. Nordic nRF52840dongle

    ```
    nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application build/zephyr/zephyr.hex --application-version 1 zephyr.zip && nrfutil dfu usb-serial -pkg zephyr.zip -p <serial-port> && rm -f zephyr.zip
    ```