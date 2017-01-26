# SpikeHome
Library to use Arduino as a home automation device 

<h1>Readme to Version 10</h1>
(Every new version that I deliver to all of my installed Arduino gets a new version, I do not have subversions by now).

<h2>Introduction</h2>
The SpikeHome library is a major part of my home automation project. My full installation currently consists of node.js, IPSymcon, an Apache web server, a MySQL Database, Z-Wave Products, FS20 Products and more and more Arduinos. I am targeting to FS20 Remote controls, Arduinos, node.js and Apache Web-Server and a non-sql database. I will reduce Z-Wave Products, IPSymcon and the MySQL Database
I will add more and more Arduino based solutions to the home automation and therefor I have built this library. 

<h2>Why Arduino?</h2>
I do not like to use available sensors anymore because: 
<ul>
<li>
Nothing I bought yet was reliable enough (365 days in a year working without any handling – reset, 100% receiving commands, 100% error correction when sending data). My Arduinos are reliable enough. They just work …
</li>
<li>Once installed you do not need to regularly update it with new software version, reboot them, … </li>
<li>The sensors are not both intelligent and remote controllable (example: you get an electric roller belt with high automation but it is not controllable by a PC)</li>
<li>You get everything you want for Arduinos, all kind of sensors, all kind of actors</li>
<li>Arduinos are good protected against hacking. If you attach them with a RS485 bus you are not able to change the code remotely.</li>
</ul>

<h2>To use the library, you need to know …</h2>
To use the library, you must be able to create Arduino sketches and include libraries in your development environment. If you use the included classes to control your sensors and actors, you only need little development experiences. If you like to develop own classes for additional sensors/actors, you need to understand the framework well and you need at least some development experiences. 

<h2>Basic design principle of my home automation</h2>
Intelligent decentral devices instead of controlling everything from a server
Automation instead of remote controlling 

<h2>Basic design principle of the library</h2>
Arduinos are capable to control many elements. They have 13 binary and 8 analog inputs and even much more if you by the “Mega”. The init/loop functions are well suited to control few elements directly. The present library is a framework that enables you to control sensors and actors in different rooms with only one Arduino and communicate to a server over an RS485 Bus or a serial/usb bus. 
This is done by an event driven design. Each sensor or actor (“object”) will be managed by a class having virtual methods that are called by the framework. 
Please use the tutorials to understand the framework starting with Tutorial1. 

<h2>The library currently supports:</h2>
<ul>
<li>Controlling binary switches (Anything that just switches on/off)</li>
<li>Controlling Roller Shutters</li>
<li>Water leak detection</li>
<li>Open/Closed Window detection</li>
<li>Movement detection</li>
<li>Controlling lights</li>
<li>Temperature</li>
<li>Humidity</li>
<li>Brightness</li>
<li>Communication with a server</li>
</ul>

<h2>Communication with PC</h2>
The library includes communication classes that can be used to communicate with the PC. You are able to add your own class and attach them to the framework. 
Currently I deliver a communication based on the serial – usb connection using a json format and a kind of toking ring communication binary protocol based on RS485. Using the RS485 solution you may attach several Arduino on one bus of two wires. 

<h2>Limits</h2>
The software design is limited by the available memory of the Arduino. I would like to have more memory and provide a better design in some places. 
Currently I do not provide the PC side of the RS485 communication. I have the source code based on node.js but it is not yet usable for others. Especially because it is closely linked to specific database tables that are historically grown.

<h2>Next topics</h2>
<ul>
<li>Adding more intelligence to the devices, sunrise/sunset detection to steer the Roller Shutters, a Clock based on the build-in features with time synchronization by the PC. Currently this is done by the PC only. </li>
<li>Add a simple server based on an Arduino instead of a PC</li>
<li>Providing node.js code to handle the RS485 based communication.</li>
</ul>
