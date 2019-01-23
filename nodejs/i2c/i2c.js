var config = require('./config.js');
var I2CIO = require('./i2cIO.js');
var i2cIO = new I2CIO();

i2cIO.openSerialPort(config.serialPort, config.baudrate, receiveDataCallback);