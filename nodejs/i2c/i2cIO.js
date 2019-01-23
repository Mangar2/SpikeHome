var SerialPort = require('serialport');
const config = require('config.js');

var I2CIO = function() {
    this.serialPort;
}

/**
 * called, once the serial port is open
 * @param {string} serialPortName name of the serial port (example "COM1")
 */
I2CIO.prototype.isOpenHandler = function(serialPortName, isOpenCallback)
{
    console.log('Serial Port ' + serialPortName + ' Opend');

    this.serialPort.on('close', function(){
        console.log("Serial port closed");
    });

    this.serialPort.on('error', function(err){
        console.log("Serial port error " + err);
    });

    isOpenCallback();
}

/**
 * Opens the serial port
 * @param {string} serialPortName name of the serial port (example "COM1")
 * @param {number} baudrate baud rate (example 9600)
 * @param {function} receiveDataCallback callback function to be called, if the com port receives data
 * @returns undefined
 */
I2CIO.prototype.openSerialPort = function (serialPortName, baudrate, receiveDataCallback) {
    var that = this;

    this.serialPort = new SerialPort(serialPortName , { baudRate: baudrate }, function(err) {
        if (err) {
            console.log('Error: ', err.message);
            console.log('Available ports: ');
            // list serial ports:
            SerialPort.list(function (err, ports) {
                ports.forEach(function (port) {
                    console.log(port.comName);
                });
            });
        }
    }); 

    this.serialPort.on('open', function() {
        that.isOpenHandler(serialPortName, receiveDataCallback);
    });

}

module.exports = I2CIO;