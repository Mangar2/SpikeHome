/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485IO.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

var SerialPort = require('serialport');
var Trace = require('./Trace');
var State = require('./RS485State');
var NetState = require('./RS485State');
var RS484Message = require('./RS485BinaryMessage');
var MessageList = require('./MessageProxy');
var state = new State();
const RS485DNS = require ('./RS485DNS');


var RS485IO = function(messageVersion, brokerPort, addresses, interfaces) {
    this.messageQueue = [];
    this.messageList = new MessageList(brokerPort);
    this.rs485DNS = new RS485DNS(addresses, interfaces);
    this.trace = new Trace();
    this.sendAmount = 0;
    // Serial port
    this.serialPort = 0;
    this.retryCount = 0;
    this.state;
    this.address;
    this.maxMessageVersion = messageVersion;
    this.curMessageVersion = messageVersion;
    this.enableChangeMessageVersion = false;
}

/**
 * Gets address and key of a command
 * @param {string} floor name of the floor
 * @param {string} room name of the room
 * @param {string} area name of the area in the room
 * @param {string} device name of the device in the room
 * @param {string} propertyName property (name) of the interface
 * @returns {object} object with address and key property
 */
RS485IO.prototype.getAddressAndKey = function(message) {
    return RS485DNS.getAddressAndKey(
        message.floor, 
        message.room, 
        message.area,
        message.interfaceName,
        message.name);
}


/**
 * Adds a message to the message tree
 * @param {object} message to add
 */
RS485IO.prototype.addMessageToMessageTree = function(message) {
    this.messageList.addMessage(message);
}


/**
 * Adds a message to the message tree
 * @param {RS484BinaryMessage} message to add
 */
RS485IO.prototype.addBinaryMessageToMessageTree = function(message) {
    var treeMessage = 
    {
        value: message.value,
        sender: message.sender,
        receiver: message.receiver,
        key: message.key,
        timestamp: message.timestamp
    }
    if (!message.hasError()) {
        treeMessage.error = message.error;
        this.rs485DNS.addSensorInfoToMessage(treeMessage);
        this.addMessageToMessageTree(treeMessage);
    }
}

/**
 * Sets the address
 * @param {number} address
 * @returns undefined
 */
RS485IO.prototype.setAddress = function (address) {
    this.address = address;
    this.state = new NetState(this.address);
}

/**
 * Call to close the serial port
 */
RS485IO.prototype.close = function () {
    this.serialPort.close(function(err) { console.log(err);});
}

/**
 * Handler called each tick to check if any message must or may be sent
 * @returns true, if any message has been sent. 
 */
RS485IO.prototype.processTick = function () {
    var messageSent = false;
    var queueMessage;
    var message = this.state.updateStateNoMessage();
    
    messageSent = this.sendMessage(message);
    if (messageSent) {
        this.messageList.updateMessageStatistic(message);
        if (message.value === state.ENABLE_SEND) {
            this.enableChangeMessageVersion = true;
        }

    }
    if (this.state.maySend) {
        queueMessage = this.sendMessageFromQueue();
        
        // Prevent to send a message twice in the same tick.
        this.state.setMaySend(false);

        if (queueMessage !== undefined) {
            this.messageList.updateMessageStatistic(queueMessage);
            messageSent = true;
        }
    }
    return messageSent;
};

/**
 * Sends the time of day in minutes as broadcast
 */
RS485IO.prototype.sendTimeOfDayInMinutes = function() {
    const BROADCAST_ADDRESS = 0;
    const REPLY = 0;
    var date = new Date(),

    timeOfDayInMinutes = date.getHours() * 60 + date.getMinutes(),
    message = new RS484Message();
    message.setVersionAndLength(this.curMessageVersion);
    message.setAddresses(1, BROADCAST_ADDRESS, REPLY);
    message.setKeyValue('C', timeOfDayInMinutes);
    this.addMessageToSendQueue(message);
}

/**
 * Opens the serial port to attach the arduino devices
 * @param {string} serialPortName name of the serial port
 * @param {number} baudrate baudrate of the serial connection
 * @param {function} openedFunction function to call if serial port is open
 */
RS485IO.prototype.openArduinoPort = function (serialPortName, baudrate, openedFunction) {
    var that = this;
    
    this.openSerialPort(serialPortName, baudrate,  function () {

        setInterval(function () {
            try {
                that.processTick(that.messageList); 
            } 
            catch (err) {
                console.log(err);
            }
        }, 100);

        setInterval(function () { 
            that.sendTimeOfDayInMinutes(); 
        }, 1000 * 60);

        that.serialPort.on('error', function(err) {
            console.log(err);
        });

        openedFunction();

    });
}


/**
 * Sets the version of messages to send
 * @param {number} version of the messages to send
 */
RS485IO.prototype.setCurMessageVersion = function(version, isRegistered) {
    if (this.enableChangeMessageVersion && version <= this.maxMessageVersion) {
        this.curMessageVersion = version;
    }
}

/**
 * processes a message handing over the token to the server
 * @param {RS485BinaryMessage} receivedMessage received token message
 */
RS485IO.prototype.processTokenMessage = function (receivedMessage) {
    var registered = this.state.isRegistered();
    var responseMessage = this.state.processIncomingStateMessage(receivedMessage);
    if (responseMessage !== undefined) {
        responseMessage = this.state.setFriendlyName(responseMessage);
        this.sendMessage(responseMessage);
    }
    if (!receivedMessage.hasError() && receivedMessage.value === state.ENABLE_SEND) {
        this.setCurMessageVersion(receivedMessage.version, registered);
    }
}


/**
 * Processes a message received:
 * 1. Trace the message
 * 2. Remove last message from messagequeue if this is the corresponding acklowledge message 
 * @param {RS485BinaryMessage} receivedMessage received message
 * @returns undefined
 */
RS485IO.prototype.processMessageRead = function (receivedMessage) {
    this.traceRead(receivedMessage);
    this.messageList.updateMessageStatistic(receivedMessage);
    if (!receivedMessage.hasError()) {
        if (receivedMessage.isResponseMessage(this.messageQueue[0])) {
            this.dequeue();
        }
        if (receivedMessage.isTokenMessage()) {
            this.processTokenMessage(receivedMessage);
        } else {
            this.addBinaryMessageToMessageTree(receivedMessage);
        }
    } else {
        console.log("error: " + receivedMessage.error);
    }
};

/**
 * Reads messages from a serial stream and stores it to a message tree
 * @param that pointer to the class object
 * @param {array} stream array of received bytes
 */
RS485IO.prototype.readAndStoreMessages = function (stream, callback) {
    var startIndex = 0;
    var message = new RS484Message();
    //console.log(stream.toString());
    startIndex = message.setFromByteStream(stream, startIndex);
    while (message.isComplete()) {
        message = this.state.setFriendlyName(message);
        this.processMessageRead(message);
        console.log(message.hexString);

        if (!message.isTokenMessage()) {
            if (!message.hasError()) {
                callback(message);
            }
        } 
        message = new RS484Message();
        startIndex = message.setFromByteStream(stream, startIndex);
    }
};

/**
 * Adds a message to the send queue. If there is already a message with the same command in the queue,
 * the message is replaced
 * @param {RS484BinaryMessage} message message to add to the queue.
 * @returns undefined
 */
RS485IO.prototype.addMessageToSendQueue = function (message) {
    var queueLength = this.messageQueue.length;
    var sameCommand = false;
    for (var queueIndex = 0; queueIndex < queueLength; queueIndex++) {
        sameCommand = message.isSameCommand(this.messageQueue[queueIndex]);
        if (sameCommand) {
            this.messageQueue[queueIndex] = message;
            break;
        }
    }
    if (!sameCommand) {
        this.messageQueue.push(message);
    }
};

/**
 * Removes the latest message from the message queue
 * @returns undefined
 */
RS485IO.prototype.dequeue = function() {
    this.messageQueue.shift();
};

/**
 * Sends a message from queue and removes it, if no reply expected
 * @returns undefined
 */
RS485IO.prototype.sendMessageFromQueue = function() {
    var messageSend;
    if (this.sendMessage(this.messageQueue[0])) {
        this.retryCount++;
        messageSend = this.messageQueue[0];
        if (this.retryCount > 10 || messageSend.reply === 0) {
            this.dequeue();
            this.retryCount = 0;
        }
    }
    return messageSend;
};

/**
 * called, once the serial port is open
 * @param {string} serialPortName name of the serial port (example "COM1")
 */
RS485IO.prototype.isOpenHandler = function(serialPortName, isOpenCallback)
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
RS485IO.prototype.openSerialPort = function (serialPortName, baudrate, receiveDataCallback) {
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

/**
 * Traces a message when reading it
 * @param {RS485BinaryMessage} message message to trace
 */
RS485IO.prototype.traceRead = function(message) {
    var traceLevel = this.trace.MAX_LEVEL;
    var traceMessage = {
        sender: message.sender,
        receiver: message.receiver,
        key: message.key,
        value: message.getValueFriendlyName(),
        hexcode: message.getHexString(),
        timestamp: message.timestamp
    }
    if (!message.isTokenMessage()) {
        traceLevel = 1;
    } else if (message.value !== state.ENABLE_SEND && message.value !== state.REGISTRATION_REQUEST) {
        traceLevel = 3;
    }
    if (message.hasError()) {
        traceLevel = 0;
        traceMessage.error = message.error;
    }

    this.trace.addTrace(traceLevel, traceMessage);
}

/**
 * Traces a message when sending it
 * @param {RS485BinaryMessage} message message to trace
 */
RS485IO.prototype.traceSend = function(message) {
    var traceLevel = this.trace.MAX_LEVEL;
    var traceMessage = {
        sender: message.sender,
        receiver: message.receiver,
        key: message.key,
        value: message.getValueFriendlyName(),
        hexcode: message.getHexString(),
        timestamp: message.timestamp
    }
    if (!message.isTokenMessage()) {
        traceLevel = 0;
    } else if (message.value !== state.ENABLE_SEND) {
        traceLevel = 3;
    }
    this.trace.addTrace(traceLevel, traceMessage);
}

/**
 * Sends a message to a serial port
 * @param {RS485BinaryMessage} message message to send
 * @param {serialport} serialPort serial port to send to
 * @returns true, if sending was successful
 */
RS485IO.prototype.sendMessage = function(message) {
    'use strict';
    var result = false;

    if (message !== undefined && message.key !== undefined) {
        if (message.version === undefined) {
            message.setVersionAndLength(this.curMessageVersion);
        }
        message.timestamp = new RS484Message().getLocalTimeAsISOString();
        this.traceSend(message);
        if (!message.isTokenMessage()) {
            console.log(message.getLogString());
        }
        var byteArray = message.getByteArray();
        this.serialPort.write(byteArray, function (err) {
            if (err) {
                console.log('Error while sending message : ' + err);
            }
            //console.log(byteArray);
        });
        this.sendAmount++;
        result = true;
    }
    return result;
};


module.exports = RS485IO;