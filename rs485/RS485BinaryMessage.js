/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485BinaryMessage.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

const NOTHING_RECEIVED = 0;
const PARTIALLY_RECEIVED = 1;
const COMLETELY_RECEIVED = 2;
const MESSAGE_SIZE_V0 = 7;
const MESSAGE_SIZE_V1 = 9;
const CCITT_CRC16_START_VALUE = 0xFFFF;
const CCITT_CRC16_POLYNOM = 0x1021;
const BITS_IN_BYTE = 8;
const BROADCAST_ADDRESS = 0;
const TOKEN = '!';

/**
 * Class to store a data packages from a network communication
 */
function RS485BinaryMessage () {
    this.state = NOTHING_RECEIVED;
    this.send = false;
    this.sender = 0;
    this.receiver = 0;
    this.reply = 0;
    this.version = undefined;
    this.lenght = undefined;
    this.key = 0;
    this.value = 0;
    this.crc16 = 0;
    this.error = undefined;
    this.hexString = "";
    this.timestamp = 0;
    this.valueFriendlyName = undefined;
    this.reason = undefined;
}

/**
 * Checks, if the current message has an error
 * @returns true, if the current message has an error
 */
RS485BinaryMessage.prototype.hasError = function() {
    return this.error !== undefined && this.error !== "";
}

/**
 * Clears the error status and set it to "no error"
 */
RS485BinaryMessage.prototype.clearError = function() {
    this.error = undefined;
}

/**
 * Sets version and length of a message
 * @param {number} version version of the message
 * @returns undefined
 */
RS485BinaryMessage.prototype.setVersionAndLength = function(version) {
    this.version = version;
    switch (version) {
        case 0: this.length = MESSAGE_SIZE_V0; break;
        case 1: this.length = MESSAGE_SIZE_V1; break;
        default: this.length = undefined;
    }
}


/**
 * Reads a new message from a stream
 * @param {array} byteArray containing the network traffic
 * @param {number} startIndex first element to concider
 * @returns {Message}
 */
RS485BinaryMessage.prototype.setFromByteStream = function(byteArray, startIndex) {
    if (typeof byteArray !== 'undefined') {
        startIndex = this.setFromByteArray(byteArray, startIndex);
    } else {
        this.clearError();
        this.hexString = "";
    }
    return startIndex;
};

/**
 * Checks, if the message is complete or still waiting for data
 * @returns {bool} true, iff the message is complete
 */
RS485BinaryMessage.prototype.isComplete = function() {
    return this.state === COMLETELY_RECEIVED;
};


/**
 * Sets sender, receiver and reply of a message
 * @param {number} sender address of the sender
 * @param {number} receiver address of the reciever
 * @param {number} reply 1, if we expect a acknoledge from the receiver, else 0
 * @returns undefined
 */
RS485BinaryMessage.prototype.setAddresses = function(sender, receiver, reply) {
    this.sender = sender;
    this.receiver = receiver;
    this.reply = reply;
};

/**
 * Sets key and value of the message
 * @param {string} key key of the message
 * @param {number} value value of the message
 * @returns undefined
 */
RS485BinaryMessage.prototype.setKeyValue = function(key, value) {
    this.key = key;
    this.value = value;
};

/**
 * Sets a reason object to the message
 * @param {object} reason description why the message happened 
 * @returns undefined
 */
RS485BinaryMessage.prototype.setReason = function(reason) {
    this.reason = reason;
};


/**
 * Sets the value of the package from a high and a low order byte
 * @param {byte} valueHigh
 * @param {byte} valueLow
 * @returns {undefined}
 */
RS485BinaryMessage.prototype.setValue = function(valueHigh, valueLow) {

    this.valueHigh = valueHigh;
    this.valueLow  = valueLow;

    if (this.key === 'h' || this.key === 't' || this.key === 's') {
        this.value = valueHigh + (valueLow / 100);
    } else {
        this.value = valueHigh * 256 + valueLow;
    }

};

/**
 * Gets the friendly name of the value. 
 * @returns the value friendly name or the value, if the friendly name is undefined
 */
RS485BinaryMessage.prototype.getValueFriendlyName = function() {
    if (this.valueFriendlyName === undefined) {
        return this.value;
    }
    return this.valueFriendlyName + " (" + this.value + ")";
}

/**
 * shifts the least significant byte to the most significant byte
 */
RS485BinaryMessage.prototype.LSByteToMSByte = function(data) {
    return (data << BITS_IN_BYTE) & 0xFF00;
}

/**
 * returns true, if the most significant bit is set
 */
RS485BinaryMessage.prototype.isMSBSet = function(data) {
    return (data & 0x8000) != 0;
}

/**
 * calculates a cyclic redundancy check value of received data
 * @param {array} byteArray array of bytes to calculate the crc value from
 * @param {number}  startIndex  index of the first value in the array to concider
 * @param {number} length amount of bytes to concider
 * @returns {number} crc16 value
 */
RS485BinaryMessage.prototype.calcCRC16 = function(byteArray, startIndex, length) {
    var crc = CCITT_CRC16_START_VALUE,
        bufferIndex,
        shiftLoop;
    for (bufferIndex = startIndex; bufferIndex < startIndex + length; bufferIndex++) {
        crc ^= this.LSByteToMSByte(byteArray[bufferIndex]);
        for (shiftLoop = 0; shiftLoop < 8; shiftLoop++) {
            if (this.isMSBSet(crc)) {
                crc = (crc << 1) ^ CCITT_CRC16_POLYNOM;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc & 0xFFFF;
}


/**
 * Sets the value of the package from a high and a low order byte
 * @param {byte} lowByte
 * @param {byte} highByte
 * @param {short} crc16 calculated cr16 value
 * @returns {undefined}
 */
RS485BinaryMessage.prototype.setCRC16 = function(lowByte, highByte, calculatedCRC16) {

    this.crc16 = (highByte << BITS_IN_BYTE) + lowByte;
    if (this.crc16 !== calculatedCRC16) {
        this.error = "CRC does not match. Expected: " +
            calculatedCRC16.toString(16) + 
            " Found: " +
            this.crc16.toString(16);
    }

};

/**
 * Calculates a parity value from 6 elements of a data field beginning with the startIndex
 * @param {array} byteArray array of 8 bit data elements
 * @param {number} startIndex first element to concider
 * @returns {Number} parity byte
 */
RS485BinaryMessage.prototype.calcParity = function (byteArray, startIndex, length) {
    'use strict';
    var i,
        parity = 0;

    if (startIndex === undefined) {
        startIndex = 0;
    }

    if (byteArray.length >= length) {
        for (i = 0; i < length; i++) {
            parity = parity ^ byteArray[i + startIndex];
        }
    }

    return parity;
};

/**
 * Sets the parity of the message based current member variables
 * @returns {undefined}
 */
RS485BinaryMessage.prototype.setParity = function() {
    var byteArray = this.getByteArray();
    this.parity = byteArray[6];
};

/**
 * Creates a debug information as string from a
 * @param {array} byteArray
 * @param {number} startIndex first element to concider
 * @returns {string} string showing legth and hex code of the byteArray
 */
RS485BinaryMessage.prototype.calcHexString = function (byteArray, startIndex, messageSize) {
    var length = byteArray.length,
        i,
        hex,
        log = "([" + length + "] ";

    for (i = 0; i < Math.min(length - startIndex, messageSize); i++) {
        hex = byteArray[i + startIndex].toString(16);
        if (hex.length < 2) {
            hex = "0" + hex;
        }
        log += " " + hex;
    }
    log +=")";
    return log;

};

/**
 * Gets a hex string representation of the message
 * @returns {string} string of hex numbers representing the message 
 */
RS485BinaryMessage.prototype.getHexString = function() {
    var byteArray = this.getByteArray();
    var hexString = this.calcHexString(byteArray, 0, this.length);
    return hexString;
}

/**
 * Sets the message information from an array of bytes received from a serial port.
 * @param {array} byteArray
 * @param {number} startIndex first element to concider
 */
RS485BinaryMessage.prototype.setV0Message = function(byteArray, startIndex) {
    var calculatedParity = this.calcParity(byteArray, startIndex, MESSAGE_SIZE_V0 - 1);
    var receivedParity = byteArray[startIndex + 5];
    this.key        = String.fromCharCode(byteArray[startIndex + 3]);
    this.setValue(byteArray[startIndex + 4], byteArray[startIndex + 5]);
    this.length = MESSAGE_SIZE_V0;
    this.timestamp = new Date().toISOString();
    if (this.sender > 127) {
        this.error = "Illegal sender address " + this.sender;
    }
    if (calculatedParity != receivedParity) {
        this.error = "Parity does not match";
    }
}

/**
 * Sets the message information from an array of bytes received from a serial port.
 * @param {array} byteArray
 * @param {number} startIndex first element to concider
 */
RS485BinaryMessage.prototype.setV1Message = function(byteArray, startIndex) {
    var crc16 = this.calcCRC16(byteArray, startIndex, MESSAGE_SIZE_V1 - 2);
    this.length     = byteArray[startIndex + 3];
    if (byteArray.length < startIndex + this.length)  {
        this.error = "insufficient data received";
    } else {
        this.key        = String.fromCharCode(byteArray[startIndex + 4]);
        this.setValue(byteArray[startIndex + 5], byteArray[startIndex + 6]);
        this.setCRC16(byteArray[startIndex + 7], byteArray[startIndex + 8], crc16);
    }
    if (this.version !== 1) {
        this.error = "Illegal message version " + this.version;
    }
    if (this.length !== 9) {
        this.error = "Illegal message length";
    }
    if (this.sender > 127) {
        this.error = "Illegal sender address " + this.sender;
    }
}

/**
 * Sets the message information from an array of bytes received from a serial port.
 * @param {array} byteArray
 * @param {number} startIndex first element to concider
 */
RS485BinaryMessage.prototype.setMessage = function(byteArray, startIndex) {
    this.sender         = byteArray[startIndex];
    this.receiver         = byteArray[startIndex + 1];
    this.reply      = byteArray[startIndex + 2] & 1;
    this.version    = byteArray[startIndex + 2] >> 1;
    this.timestamp = new Date().toISOString();

    switch (this.version) {
        case 0: this.setV0Message(byteArray, startIndex); break;
        case 1: this.setV1Message(byteArray, startIndex); break;
        default: 
            this.length = byteArray[startIndex + 3];
            this.error = "Version not supported: " + this.version;
    }

}

/**
 * Detects noise (usually received '0') and skips it
 * @param {array} byteArray
 * @param {number} startIndex first element to concider
 * @returns {number} new startIndex after noise
  */
RS485BinaryMessage.prototype.skipNoise = function(byteArray, startIndex) {
    // The RS485 USB Adapter gets zeros from time to time, if nobody is sending.
    while (byteArray.length > startIndex && byteArray[startIndex] === 0 || byteArray[startIndex] > 127) {
        startIndex++;
    }
    return startIndex;
}

/**
 * Sets the package content from an array of bytes
 * @param {array} byteArray
 * @param {number} startIndex first element to concider
 * @returns {number} first position after end of current message (= startIndex for next message)
 * @throws {string} error text
 */
RS485BinaryMessage.prototype.setFromByteArray = function(byteArray, startIndex) {
    this.clearError();
    startIndex = this.skipNoise(byteArray, startIndex);

    if (byteArray.length >= MESSAGE_SIZE_V0 + startIndex) {
        this.setMessage(byteArray, startIndex);
        this.hexString = this.calcHexString(byteArray, startIndex, this.length);
        startIndex += this.length;
        this.state = COMLETELY_RECEIVED;
    } else if (byteArray.length > startIndex)  {
        this.error = "insufficient data received";
        this.hexString = this.calcHexString(byteArray, startIndex, MESSAGE_SIZE_V0);
    } 

    return startIndex;
};

/**
 * Fills the remaining data of a version 0 message
 * @returns {Buffer} Buffer filled with message in bytecode
 */
RS485BinaryMessage.prototype.fillByteArrayV0 = function() {
    var byteArray    = new Buffer(MESSAGE_SIZE_V0);
    
    byteArray[0] = this.sender;
    byteArray[1] = this.receiver;
    byteArray[2] = this.reply + (this.version << 1);
    
    byteArray[3] = this.key.charCodeAt(0);
    byteArray[4] = this.value / 256;
    byteArray[5] = this.value % 256;
    var parity = this.calcParity(byteArray, 0, MESSAGE_SIZE_V0 - 1);
    byteArray[6] = parity;

    return byteArray;
};

/**
 * Fills the remaining data of a version 0 message
 * @returns {Buffer} Buffer filled with message in bytecode
 */
RS485BinaryMessage.prototype.fillByteArrayV1 = function() {
    var byteArray    = new Buffer(MESSAGE_SIZE_V1);

    byteArray[0] = this.sender;
    byteArray[1] = this.receiver;
    byteArray[2] = this.reply + (this.version << 1);

    byteArray[3] = MESSAGE_SIZE_V1;
    byteArray[4] = this.key.charCodeAt(0);
    byteArray[5] = this.value / 256;
    byteArray[6] = this.value % 256;
    var crc = this.calcCRC16(byteArray, 0, 7);
    byteArray[7] = crc & 0xFF;
    byteArray[8] = crc >> 8;

    return byteArray;
};

/**
 * Transforms the current message to an array of byte
 * @returns {Buffer} Buffer holding the message in bytecode
 */
RS485BinaryMessage.prototype.getByteArray = function() {
    var byteArray;
    switch (this.version) {
        case 0: byteArray = this.fillByteArrayV0(); break;
        case 1: byteArray = this.fillByteArrayV1(); break;
        default:
            throw "Unsupported message version " + this.version;
    }

    return byteArray;
};

/**
 * Returns true, if a message is a response message
 * @param {Message} message original message sent to a device
 * @returns {bool} true, if current message is a response message to the message
 * passed as parameter
 */
RS485BinaryMessage.prototype.isSameCommand = function(message) {
    // X -Kommandos werden ausgeschlossen, da sie sich zwei hintereinander folgende 'X' Kommandos ergänzen und nicht
    // ersetzen.
    return message !== undefined &&
           message.sender == this.sender &&
           message.receiver == this.receiver &&
           message.key === this.key &&
           message.key !== 'X';
};

/**
 * Returns true, if a message is a response message
 * @param {Message} message original message sent to a device
 * @returns {bool} true, if current message is a response message to the message
 * passed as parameter
 */
RS485BinaryMessage.prototype.isResponseMessage = function(message) {
    var response =
        message !== undefined &&
        message.sender == this.receiver &&
        message.receiver == this.sender &&
        message.key === this.key &&
        message.value == this.value &&
        message.reply === 1;
    return response;
};

RS485BinaryMessage.prototype.getLogString = function () {
    'use strict';
    var now = new Date();
    var log = now.toLocaleTimeString() + " ";
    if (!this.hasError()) {
        log += this.sender + " => " + this.receiver + " (r:" + this.reply + "): " + this.key + " == " + this.value;
        while (log.length < 35) {
            log += " ";
        }
        if (this.hexString !== "") {
            log += this.hexString;
        } else {
            log += this.getHexString();
        }
        log += " (t:" + statistic.total + " !:" + statistic.token + " p:" + statistic.parityError;
        log += " l:" + statistic.insufficentData + " o:" + statistic.notForMe + ")";
    } else {
        log += this.error + " " + this.hexString;
    }
    return log;
};

/**
 * Checks, if the message is addressed to this server
 * @returns true, if the message is addressed to this server
 */
RS485BinaryMessage.prototype.isForMe = function(myAddress) {
    'use strict';
    return this.receiver === BROADCAST_ADDRESS || this.receiver === myAddress;
};

/**
 * Checks, if the message is a token handling message
 * @returns true, if the message is only managing token handling
 */
RS485BinaryMessage.prototype.isTokenMessage = function() {
    return this.key == TOKEN;
}


module.exports = RS485BinaryMessage;