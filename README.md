# ESPHome Modbus configuration for Schneider iEM3155 Energy Meter v1.3

![iem3155](https://github.com/Gaudi111/iem3155_esphome/blob/main/iem3155.png)

### Revision:
- **v1.3** (2023-09-08) by Gaudi111
   * Code adaptation for ESP8266 NodeMCU SBC + generic RS485 to TTL converter, including hardware wiring and configuration files.
   * Added full set of registers as per Schneider Documentation
   * Added external function (to be used as lambda function in text_sensors) to convert DATETIME data format to human readable format.

- **v1.2** (2023-09-08)
   * Further code revisions to avoid wrong initial values on reboots (not power cycles)
   * Minor fixes
     * Substitutions, icons, diagnostic sensors etc.

- **v1.1** (2023-07-17)
   * Improved documentation
   * Added two additional (calculated) sensors. The daily solar power self-consumption sensor rely on daily yield value from my solar inverter via a Home Assistant sensor 
      * Daily solar power self-consumption sensor
      * Daily export power sensor
    * Code revised to be more resilient to reboots (not power cycles)
      * Daily export power base value is now retained in RTC memory
        
This repo contains my production configuration and serves as a complete ESPHome example for the iEM3155 Energy Meter.

Extremely easy, full Home Assistant integration using ESPHome as wireless Modbus node.

## iEM3155 ESPHome sensors exposed to Home Assistant:
![iEM3155 ESPHome HA sensors](https://github.com/Gaudi111/iem3155_esphome/blob/main/iem3155_HA_sensors.PNG)![iEM3155 ESPHome HA diagnostic sensors](https://github.com/htvekov/iem3155_esphome/blob/main/iem3155_HA_sensors1.PNG)

## iEM3155 device configuration:
* Serial: 19200, EVEN, 1 (default)
* Slave address: 0x01

> ***Note***
> 
> Ensure that Com.Protection in the device is *DISABLED* - Com.Protection is *ENABLED* by default !!
>
> Otherwise partial reset of daily energy import registers will be *IGNORED* by the device (id: daily_energy_import_total + 3 x daily_energy_import_lX)

## ESP8266 module:
[ESP8266 NodeMCU](https://www.amazon.com/Development-Internet-Communication-Compatible-Micropython/dp/B08MPK8SXC?th=1)

![ESP8266 NodeMCU](https://github.com/Gaudi111/iem3155_esphome/blob/main/SBC-NodeMCU-2.png)

## RS485 modbus module
* [MAX485 RS485 Transceiver Module TTL UART Serial to RS-485 Module](https://www.amazon.com/ANMBEST-Transceiver-Arduino-Raspberry-Industrial-Control/dp/B088Q8TD4V)
* Simple, cheap module
* Supports both 3.3 and 5v
![TTL/RS485 module](https://github.com/Gaudi111/iem3155_esphome/blob/main/RS485-TTL_Module.webp)

## RS485 <-> NodeMCU Wiring 
<pre>
TTL-RS485 Module  |  ESP8266  |  
  VCC                  3V (in my setup it works fine @3.3V, so no need for 5V)
  GND                  GND
  DE-RE                D1 / GPIO05 (DE & RE Pins need to be wired together)
  DI                   TX / GPIO01 (Hardware control for better stability, make sure to disable logging, see comment in .yaml)
  R0                   RX / GPIO03
</pre>

> ***Note***
> 
> This setup uses RX/TX UART Hardware Control pins for better stability, make sure to disable logging in the logger section of ESPHome yaml configuration, as the commands sent from serial logging may interfere with ModBus communication
>
>  
```YAML
  logger:
    baud_rate: 0
```



## RS485 <-> Schneider iEM3155 Meter Wiring
<pre>
TTL-RS485 Module  |  iEM3155  |  
  Data B                D0/-
  Data A                D1/+
</pre>

## Additional information
The iEM3155 device also offers some additional interesting features that might be useful for others. Tariff/tariff rates and overload alarm. There's also both a digital input and output port that can be utilized as well.

Check the iEM3155 user manual/technical [pdf datasheet](https://www.productinfo.schneider-electric.com/iem3100_iem3200_iem3300/iem3100_iem3200_iem3300-series_user-manual/iEM3100_iEM3200_iEM3300%20series_User%20Manual/English/DOCA0005EN-15.pdf) or [online version](https://www.productinfo.schneider-electric.com/iem3100_iem3200_iem3300/iem3100_iem3200_iem3300-series_user-manual/iEM3100_iEM3200_iEM3300%20series_User%20Manual/English/BM_iEM3100_iEM3200_iEM3300SeriesUserManual_0000351220.ditamap.xml) for further info.
