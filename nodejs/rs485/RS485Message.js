/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485Message.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

var StatisticData = function() {
    this.total = 0;
    this.token = 0;
    this.parityError = 0;
    this.insufficentData = 0;
    this.notForMe = 0;
    this.CCITT_CRC16_START_VALUE = 0xFFFF;
    this.CCITT_CRC16_POLYNOME = 0x1021;
    this.BITS_IN_BYTE = 8;

};

var statistic = new StatisticData();

const NOTHING_RECEIVED = 0;
const PARTIALLY_RECEIVED = 1;
const COMLETELY_RECEIVED = 2;

/**
 * Class to store a data packages from a network communication
 */
function RS485Message () {
    this.state = NOTHING_RECEIVED;
    this.broadCase = 0;
    this.sender = 0;
    this.receiver = 0;
    this.reply = 0;
    this.key = 0;
    this.value = 0;
    this.parity = 0;
    this.crc16 = 0;
    this.error = "";
    this.JSONString = "";
    this.timestamp = 0;
}

RS485Message.prototype.isComplete = function() {
    return this.state === COMLETELY_RECEIVED;
};

RS485Message.prototype.setAddresses = function(sender, target, sendResponse) {
    this.sender = sender;
    this.target = target;
    this.sendResponse = sendResponse;
};

RS485Message.prototype.setCommand = function(command, value) {
    this.key = command;
    this.value = value;
};


/**
 * Sets the value of the package from a high and a low order byte
 * @param {byte} valueHigh
 * @param {byte} valueLow
 * @returns {undefined}
 */
RS485Message.prototype.setValue = function(valueHigh, valueLow) {

    this.valueHigh = valueHigh;
    this.valueLow  = valueLow;

    if (this.key === 'h' || this.key === 't' || this.key === 's') {
        this.value = valueHigh + (valueLow / 100);
    } else {
        this.value = valueHigh * 256 + valueLow;
    }

};


RS485Message.prototype.LSByteToMSByte = function(data) {
    return data << BITS_IN_BYTE;
}

RS485Message.prototype.isMSBSet = function(data) {
    return (data & 0x8000) != 0;
}

RS485Message.prototype.crc16 = function(byteArray, length) {
    var crc = CCITT_CRC16_START_VALUE,
        bufferIndex,
        shiftLoop;
    for (bufferIndex = 0; bufferIndex < length; bufferIndex++) {
        crc ^= LSByteToMSByte(byteArray[bufferIndex]);
        for (shiftLoop = 0; shiftLoop < 8; shiftLoop++) {
            if (isMSBSet(crc)) {
                crc = (crc << 1) ^ CCITT_CRC16_Polynome;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}


/**
 * Searches for a byte in a stream
 * @param {array} stream stream of input data
 * @param {number} startIndex first element to concider
 * @returns {number} index of the first occurence or "undefined" if not found
 */
RS485Message.prototype.searchByteStream = function(stream, startIndex, searchFor) {
    var found;
    if (startIndex !== undefined) {
        for(var curIndex = startIndex; stream.length > curIndex; curIndex++) {
            if (stream[curIndex] === searchFor) {
                found = curIndex;
                break;
            }

        }
    }
    return found;
};

/**
 * Searches the first element of a message
 * @param {array} stream stram of input data 
 */
RS485Message.prototype.searchFirstMessageElement = function(stream) {
    var firstElement;
    if (this.state === NOTHING_RECEIVED) {
        firstElement = this.searchByteStream(stream, 0, '{'.charCodeAt(0));
        this.state = PARTIALLY_RECEIVED;
    } else if (this.state === PARTIALLY_RECEIVED) {
        firstElement = 0;
    }
    return firstElement;
}

/**
 * Searches the last element of a message
 * @param {array} stream stram of input data 
 */
RS485Message.prototype.searchLastMessageElement = function(stream, startIndex) {
    var lastElement;
    if (startIndex !== undefined) {
        lastElement = this.searchByteStream(stream, startIndex, '}'.charCodeAt(0));
        if (lastElement !== undefined) {
            this.state = COMLETELY_RECEIVED;
        } else {
            lastElement = stream.length - 1;
        }
    }
    return lastElement;
}

/**
 * Sets the message from an json object received from serial
 * @param object Object containing a serial representation of the message
 */
RS485Message.prototype.setFromObject = function(object) {
    this.sender = object.S;
    this.receiver = object.R;
    this.reply = object.A;
    this.key = object.K;
    this.value = object.V;
    this.crc16 = parseInt(object.C);
    this.timestamp = new Date().toISOString();
}

/**
 * Sets the package content from a stream formatted as JSON
 * @param {array} stream stram of input data 
 * @returns {undefined}
 * @throws {string} error text
 */
RS485Message.prototype.setFromStreamJSON = function(stream) {
    var firstElement = this.searchFirstMessageElement(stream);
    var lastElement = this.searchLastMessageElement(stream, firstElement);

    for (var curElement = firstElement; curElement <= lastElement; curElement++ ) {
        this.JSONString += String.fromCharCode(stream[curElement]);
    }

    if (this.state === COMLETELY_RECEIVED) {
        var object = JSON.parse(this.JSONString);
        this.setFromObject(object);
    } 
};

module.exports = RS485Message;