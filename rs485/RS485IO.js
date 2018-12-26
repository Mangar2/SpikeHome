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

var NodeModulePath = 'C:/Program Files/nodejs/node_modules';
var SerialPort = require(NodeModulePath + '/serialport');
var Trace = require('./Trace');
var State = require('./RS485State');
var NetState = require('./RS485State');
var MessageList = require('./MessageList');
var state = new State();

var RS485IO = function(messageVersion) {
    this.messageQueue = [];
    this.messageList;
    this.trace = new Trace();
    this.sendAmount = 0;
    // Serial port
    this.serialPort = 0;
    this.retryCount = 0;
    this.state;
    this.address;
    this.maxMessageVersion = messageVersion;
    this.curMessageVersion = messageVersion;
}

/**
 * Initializes a tree of message information
 * @param {object} addresses configuration of all addresses used in messages
 * @param {object} interfaces configuration of all interfaces used in messages
 * @returns undefined
 */
RS485IO.prototype.initMessageList = function (addresses, interfaces) {
    this.messageList = new MessageList(addresses, interfaces);
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
    }
    if (this.state.maySend) {
        queueMessage = this.sendMessageFromQueue();
        if (queueMessage !== undefined) {
            this.messageList.updateMessageStatistic(queueMessage);
            messageSent = true;
        }
    }
    return messageSent;
};

/**
 * Sets the version of messages to send
 * @param {number} version of the messages to send
 */
RS485IO.prototype.setCurMessageVersion = function(version) {
    if (version <= this.maxMessageVersion) {
        this.curMessageVersion = version;
    }
}

/**
 * processes a message handing over the token to the server
 * @param {RS485BinaryMessage} receivedMessage received token message
 */
RS485IO.prototype.processTokenMessage = function (receivedMessage) {
    var responseMessage = this.state.processIncomingStateMessage(receivedMessage);
    if (responseMessage !== undefined) {
        responseMessage = this.state.setFriendlyName(responseMessage);
        this.sendMessage(responseMessage);
    }
    if (!receivedMessage.hasError() && receivedMessage.value === state.ENABLE_SEND) {
        this.setCurMessageVersion(receivedMessage.version);
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
    }
    if (receivedMessage.isTokenMessage()) {
        this.processTokenMessage(receivedMessage);
    } else {
        this.messageList.addMessage(receivedMessage);
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
    this.retryCount = 0;
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
    this.serialPort = new SerialPort(serialPortName , { baudrate: baudrate }, function(err) {
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
        traceLevel = 1;
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
        message.timestamp = new Date().toISOString();
        this.traceSend(message);
        if (!message.isTokenMessage()) {
            console.log(message.getLogString());
        }
        this.serialPort.write(message.getByteArray(), function (err, result) {
            if (err) {
                console.log('Error while sending message : ' + err);
            }
        });
        this.sendAmount++;
        result = true;
    }
    return result;
};


module.exports = RS485IO;