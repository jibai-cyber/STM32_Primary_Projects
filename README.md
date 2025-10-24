# STM32_Primary_Projects

## What are these about?  
### Low-precision oscilloscope
#### description
One of the projects in a small competition that I found quite interesting.  

#### Requirement
> https://gitee.com/liu-mouyi/automated/tree/master/

#### Peripheral devices
- DMA
- ADC
- OLED
- KEY

#### Methods included
Including but not limited to:
- Coordination of sampling frequency and data processing frequency
- Enable the signal to be updated with low latency and displayed on the OLED
- Capture of the peak voltage and frequency of the sine wave signal
- Selection of the appropriate scaling ratio

### Smart Belt
#### description
My first team collaboration project and this is the code for the embedded part.  
A belt with functions including calling, positioning, alarming and etc.

#### Requirement
See the uploaded file

#### Peripheral devices
Including but not limited to:
- atgm336GPS
- MPU6050
- ESP8266 (Wi-Fi module)
- SYN6288
- OLED

#### Methods included
Including but not limited to:
- Interpreting NMEA messages and obtaining the location information
- Determine whether the person has fallen by using the acceleration sensor
- Communicating with the backend and parsing JSON messages
- Broadcasting the text messages sent by the backend in Chinese using SYN6288
- The design of multiple menus
- Power-off storage of information (FLASH)

Below are some demonstration videos that I recorded myself. The extraction code is: 02hc

> https://www.alipan.com/s/T4jDgnfJMRZ

Including the results of each project.  
Unfortunatly, The video of _Low-precision oscilloscope_ is missed.  
But I assure you that the code works good (Since I got full marks).

## To-do task
1. The code needs to be cleaned.
2. Provide a instruction Doc.

## Project start date
March 2023 -> _Smart Belt_  
May 2023 -> _Low-precision oscilloscope_

## Appreciation
For the laboratory seniors and mentors who introduced me to this field.

