# SpikeHome

Library to use Arduino as a home automation device

## Introduction

The SpikeHome library is a major part of my home automation project. My full installation currently consists of node.js, IPSymcon, an Apache web server, a MySQL Database, Z-Wave Products, FS20 Products and more and more Arduinos. I am targeting to FS20 Remote controls, Arduinos, node.js and Apache Web-Server and a non-sql database. I will reduce Z-Wave Products, IPSymcon and the MySQL Database
I will add more and more Arduino based solutions to the home automation and therefor I have built this library.

## Content

* The library consists of the basic framework found in "SpikeHome" (no additional libraries needed)
* Additional sensors found in "SpikeSensors" (needs additional libraries to support the sensors)
* Tutorials found in "Tutorial" that you should use to learn how the library works

## Version history

* 1.0 Initial Version
* 1.1 Implemented a new message format. It now has a length of 9 bytes: Sender-Address, Receiver-Address, Version/Reply, Message-Length in Bytes, Key (Id), Value (2 bytes), CRM (2 bytes). It adds a version information in the upper 7 bits of the second byte, a message-length byte and it replaces the parity byte by a CRC-16.

## Why Arduino

I do not like to use available sensors anymore because:

* Nothing I bought yet was reliable enough (365 days in a year working without any handling – reset, 100% receiving commands, 100% error correction when sending data). My Arduinos are reliable enough. They just work …
* Once installed you do not need to regularly update it with new software version, reboot them, …
* The sensors are not both intelligent and remote controllable (example: you get an electric roller belt with high automation but it is not controllable by a PC)
* You get everything you want for Arduinos, all kind of sensors, all kind of actors
* Arduinos are good protected against hacking. If you attach them with a RS485 bus you are not able to change the code remotely.

## To use the library, you need to know …

To use the library, you must be able to create Arduino sketches and include libraries in your development environment. If you use the included classes to control your sensors and actors, you only need little development experiences. If you like to develop own classes for additional sensors/actors, you need to understand the framework well and you need at least some development experiences.

## Basic design principle of my home automation

Intelligent decentral devices instead of controlling everything from a server
Automation instead of remote controlling

## Basic design principle of the library

Arduinos are capable to control many elements. They have 13 binary and 8 analog inputs and even much more if you by the “Mega”. The init/loop functions are well suited to control few elements directly. The present library is a framework that enables you to control sensors and actors in different rooms with only one Arduino and communicate to a server over an RS485 Bus or a serial/usb bus.
This is done by an event driven design. Each sensor or actor (“object”) will be managed by a class having virtual methods that are called by the framework.
Please use the tutorials to understand the framework starting with Tutorial1.

## The library currently supports

* Controlling binary switches (Anything that just switches on/off)
* Controlling Roller Shutters
* Water leak detection
* Open/Closed Window detection
* Movement detection
* Controlling lights
* Temperature
* Humidity
* Brightness
* Communication with a server

## Communication with PC

The library includes communication classes that can be used to communicate with the PC. You are able to add your own class and attach them to the framework.
Currently I deliver a communication based on the serial – usb connection using a json format and a kind of toking ring communication binary protocol based on RS485. Using the RS485 solution you may attach several Arduino on one bus of two wires.

## Limits

The software design is limited by the available memory of the Arduino. I would like to have more memory and provide a better design in some places.
Currently I do not provide the PC side of the RS485 communication. I have the source code based on node.js but it is not yet usable for others. Especially because it is closely linked to specific database tables that are historically grown.

## Node-Js Version 0.1

I have added node.js sources to connect to the RS485 BUS. It works already well, but I am not fully happy with the archtitecture and it contains some compatibility code (like a call to store the data in an SQL server). I need to remove this from the library and put it in the main file (supporting a "persistence callback"). Still it will work and writes some error messages to stdio.
The Data of the arduinos is only held in memory. The library does not include any persistence. Persistence is a topic that should be realized in a different library or even in a different micro-service. The data in memory can be fetched and modified by a small REST API (having very poor error handling yet).

## Next topics

* Adding more intelligence to the devices, sunrise/sunset detection to steer the Roller Shutters, a Clock based on the build-in features with time synchronization by the PC. Currently this is done by the PC only.
* Add a simple server based on an Arduino instead of a PC
* Providing node.js code to handle the RS485 based communication.
