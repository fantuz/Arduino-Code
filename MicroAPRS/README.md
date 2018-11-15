MicroAPRS
==========

MicroAPRS is an APRS firmware for [MicroModem](http://unsigned.io/micromodem). It supports both normal KISS mode, and a simple serial protocol for easy communication with an Arduino, or other MCU.

You can buy a complete modem from [my shop](http://unsigned.io/shop), or you can build one yourself pretty easily. Take a look at the documentation in the [MicroModem](https://github.com/markqvist/MicroModem) repository for information and getting started guides!

## Some features

- Send and receive AX.25 APRS packets
- Full modulation and demodulation in software
- Easy configuration of callsign and path settings
- Flexibility in how received packets are output over serial connection
- Persistent configuration stored in EEPROM
- Shorthand functions for sending location updates and messages, so you don't need to manually create the packets
- Ability to send raw packets
- Support for settings APRS symbols
- Support for power/height/gain info in location updates
- Ability to automatically ACK messages adressed to the modem
- Can run with open squelch
- Supports KISS mode for use with programs on a host computer

## KISS mode

When the modem is running in KISS mode, there's really not much more to it than connecting the modem to a computer, opening whatever program you want to use with it, and off you go.

## Modem control - SimpleSerial

If you want to use the SimpleSerial protocol, here's how to control the APRS modem over a serial connection. The modem accepts a variety of commands for setting options and sending packets. Generally a command starts with one or more characters defining the command, and then whatever data is needed to set the options for that command. Here's a list of the currently available commands:

## Serial commands

Command | Description
--- | :---
__!\<data>__  | Send raw packet
__@\<cmt>__ | Send location update (cmt = optional comment)
__#\<msg>__ | Send APRS message
&nbsp; | &nbsp;
__c\<call>__ |  Set your callsign
__d\<call>__ |  Set destination callsign
__1\<call>__ |  Set PATH1 callsign
__2\<call>__ |  Set PATH2 callsign
&nbsp; | &nbsp;
__sc\<ssid>__ | Set your SSID
__sd\<ssid>__ | Set destination SSID
__s1\<ssid>__ | Set PATH1 SSID
__s2\<ssid>__ | Set PATH2 SSID
&nbsp; | &nbsp;
__lla\<LAT>__ | Set latitude (NMEA-format, eg 4903.50N)
__llo\<LON>__ | Set latitude (NMEA-format, eg 07201.75W)
__lp\<0-9>__  | Set TX power info
__lh\<0-9>__  | Set antenna height info
__lg\<0-9>__  | Set antenna gain info
__ld\<0-9>__  | Set antenna directivity info
__ls\<sym>__  | Select symbol
__lt\<s/a>__  | Select symbol table (standard/alternate)
&nbsp; | &nbsp;
__mc\<call>__ | Set message recipient callsign
__ms\<ssid>__ | Set message recipient SSID
__mr\<ssid>__ | Retry last message
__ma\<1/0>__  | Automatic message ACK on/off
&nbsp; |&nbsp;
__ps\<1/0>__  | Print SRC on/off
__pd\<1/0>__  | Print DST on/off
__pp\<1/0>__  | Print PATH on/off
__pm\<1/0>__  | Print DATA on/off
__pi\<1/0>__  | Print INFO on/off
__v\<1/0>__ | Verbose mode on/off
__V\<1/0>__ | Silent mode on/off
&nbsp; | &nbsp;
__w\<XXX>__ | Set preamble in ms
__W\<XXX>__ | Set TX tail in ms
&nbsp; | &nbsp;
__S__ | Save configuration
__L__ | Load configuration
__C__ | Clear configuration
__H__ | Print configuration



### Examples

__To set your callsign to XX1YYY-5, and then save the configuration, send these three commands:__
```
cXX1YYY
sc5
S
```

__To send an APRS message to ZZ5ABC-1 with the content "Hi there!", send these commands:__
```
mcZZ5ABC
ms1
# Hi there!
```

__To send a location update, with the comment "MicroAPRS", you can do something like this:__
```
lla5230.70N
llo01043.70E
@MicroAPRS
```

__To send an APRS message to ZZ5ABC-1 with the content "Hi there!", using a raw packet, send this command:__
```
!:ZZ5ABC-1 :Hi there!{01
```
__Here's an example of how to send a location update with power, height and gain information, using a raw packet:__
```
!=5230.70N/01043.70E-PHG2410MicroAPRS
```

### EEPROM Settings
When saving the configuration, it is written to EEPROM, so it will persist between poweroffs. If a configuration has been stored, it will automatically be loaded when the modem powers up. The configuration can be cleared by sending the "clear configuration" command (`C`).

### Serial Connection

To connect to the modem use __9600 baud, 8N1__ serial. By default, the firmware uses time-sensitive input, which means that it will buffer serial data as it comes in, and when it has received no data for a few milliseconds, it will start interpreting whatever it has received. This means you need to set your serial terminal program to not send data for every keystroke, but only on new-line, or pressing send or whatever. If you do not want this behaviour, you can compile the firmware with the DEBUG flag set, which will make the modem wait for a new-line character before interpreting the received data. I would generally advise against this though, since it means that you cannot have newline characters in whatever data you want to send!

![MicroModem](https://raw.githubusercontent.com/markqvist/MicroModem/master/Design/Images/1.jpg)

The project has been implemented in your normal C with makefile style, and uses AVR Libc. The firmware is compatible with Arduino-based products, although it was not written in the Arduino IDE.

Visit [my site](http://unsigned.io) for questions, comments and other details.
