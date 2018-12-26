/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      MessageList.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

var AddressMap = require ("./AddressMap.js");
var InterfaceMap = require("./InterfaceMap.js");

function MessageList (addresses, interfaces) {
    this.messages = {};
    this.statistic = { 
        send: {
            totalMessages: 0,
            dataMessages: 0, 
            tokenMessages: 0, 
            errors: {total: 0}
        },
        received: {
            totalMessages: 0,
            dataMessages: 0, 
            tokenMessages: 0, 
            errors: {total: 0}
        }
    };
    this.addressMap = new AddressMap(addresses);
    this.interfaceMap = new InterfaceMap(interfaces);
    this.total = 0;
    this.token = 0;
    this.parityError = 0;
    this.insufficentData = 0;
    this.notForMe = 0;
    this.CCITT_CRC16_START_VALUE = 0xFFFF;
    this.CCITT_CRC16_POLYNOME = 0x1021;
    this.BITS_IN_BYTE = 8;
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
MessageList.prototype.getAddressAndKey = function(floor, room, area, device, propertyName) {
    var address = this.addressMap.getDeviceAddress(floor, room, area);
    var key = this.interfaceMap.getKey(device, propertyName);
    return {
        address: address, 
        key: key
    }
}

/**
 * Savely gets a property from an object. 
 * @param {object} object to get entry from
 * @param {index} propertyName name of the property to retrieve
 * @param {anything} addInfUndefine property to add, if there is no entry
 * @result property of the object or undefined, if the property is not retrievable
 */
MessageList.prototype.getFromObject = function(object, propertyName, addIfUndefined) {
    var result;
    if (object !== undefined && propertyName !== undefined) {
        if (object[propertyName] === undefined && addIfUndefined !== undefined) {
            object[propertyName] = addIfUndefined;
        }
        result = object[propertyName];
    }
    return result;
}

/**
 * Adds a Message to a floor message structure
 * @param {object} floorMessages hirachical structure to store all messages of a floor
 * @param {object} addressInfo information to the current address of the message
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToFloor = function(floorMessages, addressInfo, message) {
    var roomMessages = this.getFromObject(floorMessages, addressInfo.room, {});
    this.addMessageToRoom(roomMessages, addressInfo, message);
}

/**
 * Adds a Message to a room messages structure
 * @param {object} roomMessages hirachical structure to store all messages of a room
 * @param {object} addressInfo information to the current address of the message
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToRoom = function(roomMessages, addressInfo, message) {
    var areaMessages = this.getFromObject(roomMessages, addressInfo.area, {});
    this.addMessageToArea(areaMessages, message);
}

/**
 * Adds a Message to an area messages structure
 * @param {object} areaMessages hirachical structure to store all messages of an area
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToArea = function(areaMessages, message) {
    var interfaceProperty = this.interfaceMap.getPropertyFromKey(message.key);
    var deviceMessages = this.getFromObject(areaMessages, interfaceProperty.interfaceName, {});
    this.addMessageToDevice(deviceMessages, interfaceProperty, message);
}

/**
 * Adds a Message to a device messages structure
 * @param {object} deviceMessages hirachical structure to store all messages of a device
 * @param {object} interfaceProperty interface property information
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToDevice = function(deviceMessages, interfaceProperty, message) {
    var sensorMessages = this.getFromObject(deviceMessages, interfaceProperty.name, {"current":undefined, "history": []});
    this.addMessageToSensor(sensorMessages, message);
}

/**
 * Update the statistic of a message tree node
 * @param {object} sensorMessages hirachical structure to store all messages of a sensor
 * @param {object} message message used to update the statistic
 */
MessageList.prototype.indentifyUnexpectdChange = function(sensorMessages) {
    var history = sensorMessages.history;
    var current = sensorMessages.current;
    if (history !== undefined && current !== undefined) {
        var currentValue = sensorMessages.current.value;
        var change = 0;
        for (historyIndex = 1; historyIndex < history.length; historyIndex++) {

        }
    }
}

/**
 * Update the statistic of a message tree node
 * @param {object} sensorMessages hirachical structure to store all messages of a sensor
 * @param {object} message message used to update the statistic
 * @returns {string|undefined} event text based on statistic information
 */
MessageList.prototype.updateStatistic = function(sensorMessages, message) {
    var statistics;
    var newValue = message.value;
    var valueChange = sensorMessages.current === undefined ? 0: sensorMessages.current.value - newValue;
    var valueChangeSquare = valueChange * valueChange;
    var result;
    if (sensorMessages.statistics === undefined) {
        sensorMessages.statistics = {
            send: 0,
            received: 0,
            changes: 0,
            valueChangeSquares: 0,
            maxValue: newValue,
            minValue: newValue
        }
    }
    statistics = sensorMessages.statistics;
    if (message.send) {
        statistics.send++;
    } else {
        statistics.received++;
    }
    if (valueChange != 0) {
        statistics.changes++;
        statistics.valueChangeSquares += valueChangeSquare;
        if ((statistics.valueChangeSquares/statistics.changes) * 1.5 < valueChangeSquare) {
            result = "high change";
        }
    }
    statistics.maxValue = Math.max(statistics.maxValue, newValue);
    statistics.minValue = Math.min(statistics.minValue, newValue);
    return result;
}

/**
 * Adds a Message to a sensor messages structure
 * @param {object} sensorMessages hirachical structure to store all messages of a sensor
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToSensor = function(sensorMessages, message) {
    var history = sensorMessages.history;
    var event = this.updateStatistic(sensorMessages, message);
    if (message.send === true) {
        sensorMessages.command = {
            "value" : message.getValueFriendlyName(),
            "timestamp" : message.timestamp,
            "reason" : message.reason
        }
    } else {
        sensorMessages.current = {
            "value" : message.getValueFriendlyName(),
            "target": message.receiver,
            "error" : message.error,
            "timestamp": message.timestamp
        };
        if (event !== undefined) {
            sensorMessages.current.event = event;
        }
        if (history.length === 0 || history[0].value !== message.value) {
            sensorMessages.history = [sensorMessages.current].concat(history);
        }
    }
}

/**
 * Updates a statistic about messages
 * @param {RS485BinaryMessage} message message to include in the statistic
 * @returns undefined
 */
MessageList.prototype.updateMessageStatistic = function(message) {
    var statistic = this.statistic[message.send? "send" : "received"];

    statistic.totalMessages++;

    if (!message.hasError()) {

        if (message.isTokenMessage()) {
            statistic.tokenMessages++;
        } else {
            statistic.dataMessages++;
        }
    } else {
        statistic.errors.total++;
        if (statistic.errors[message.error] === undefined) {
            statistic.errors[message.error] = 1;
        } else {
            statistic.errors[message.error]++;
        }
    }

}

/**
 * Adds a message a hirachical data store
 * @param {RS485BinaryMessage} message message to add
 * @returns undefined
 */
MessageList.prototype.addMessage = function(message) {
    var address = message.send ? message.receiver : message.sender;

    if (!message.hasError()) {
        var location = this.addressMap.getLocationFromAddress(address);
        var floorMessages = this.getFromObject(this.messages, location.floor, {});
        this.addMessageToFloor(floorMessages, location, message);
    }
};

/**
 * Gets the message statistics as HTTPMessage
 * @param {object} res HTTP result object
 * @returns undefined
 */
MessageList.prototype.getStatisticsAsHTTPMessage = function (res) {
    res.writeHead(200, {'Content-Type': 'text/plain'});
    var payload = this.statistic;
    res.end(JSON.stringify(payload));
}

/**
 * Gets all infos related to a room
 * @param {string} floor name of the floor
 * @param {string} room name of the room
 */
MessageList.prototype.getRoomData = function (floor, room) {
    var result = {};
    var floorMessages = this.messages[floor];
    if (floorMessages !== undefined) {
        var roomMessages = floorMessages[room];
        if (roomMessages != undefined) {
            result = roomMessages;
        }
    }
    return result;
}

/**
 * Gets all infos related to an area
 * @param {string} floor name of the floor
 * @param {string} room name of the room
 * @param {string} area name of the area in the room
 */
MessageList.prototype.getAreaData = function (floor, room, area) {
    var roomMessages = this.getRoomData(floor, room);
    var result = {};
    if (roomMessages !== undefined) {
        var areaMessages = roomMessages[area];
        if (areaMessages != undefined) {
            result = areaMessages;
        }
    }

    return result;
}

/**
 * Gets all infos related to a device 
 * @param {string} floor name of the floor
 * @param {string} room name of the room
 * @param {string} area name of the area in the room
 * @param {string} device name of the device in the room
 */
MessageList.prototype.getDeviceData = function (floor, room, area, device) {
    var areaMessages = this.getAreaData(floor, room, area);
    var result = {};
    if (areaMessages !== undefined && areaMessages[device] !== undefined) {
        result = areaMessages[device];
    }

    return result;
}

/**
 * Gets all infos related to an interface of a device 
 * @param {string} floor name of the floor
 * @param {string} room name of the room
 * @param {string} area name of the area in the room
 * @param {string} device name of the device in the room
 * @param {string} interfaceProperty property (name) of the interface
 */
MessageList.prototype.getPropertyData = function (floor, room, area, device, interfaceProperty) {
    var deviceMessages = this.getDeviceData(floor, room, area, device);
    var result = {};

    if (deviceMessages !== undefined && deviceMessages[interfaceProperty] !== undefined) {
        result = deviceMessages[interfaceProperty];
    }

    return result;
}

/**
 * Gets a list of links leading to all stored floors in the message tree
 * @param {string} baseLink base link to the message tree
 */
MessageList.prototype.getFloorLinks = function (baseLink) {
    var floorLinks = [];
    for (var floor in this.messages) {
        floorLinks.push(baseLink + floor);
    }
    return floorLinks;
}

/**
 * Gets a list of links leading to all stored rooms of a floor in the message tree
 * @param {string} baseLink base link to the message tree
 * @param {string} floor name of the floor to search for rooms
 */
MessageList.prototype.getRoomLinks = function (baseLink, floor) {
    var roomLinks = [];
    if (this.messages[floor] !== undefined) {
        for (var room in this.messages[floor]) {
            roomLinks.push(baseLink + floor + "/" + room);
        }
    }
    return roomLinks;
}


module.exports =  MessageList;