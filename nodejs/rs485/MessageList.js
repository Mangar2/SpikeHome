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

var fs = require("fs");

function MessageList (fileName) {
    this.messages = {};
    this.fileName = fileName;
    if (this.fileName === undefined) {
        this.fileName = "./data/messageTree.json";
    }
    this.readTree();
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
}

/**
 * Saves the tree to a file (existing file is overwritten)
 */
MessageList.prototype.saveTree = function() {
    fs.writeFile(this.fileName, JSON.stringify(this.messages), function(err){
        if (err) {
            console.error(err);
        }
    });
}

/**
 * Reads the message tree from a file
 */
MessageList.prototype.readTree = function() {
    try {
        var contents = fs.readFileSync(this.fileName);
        if (contents !== undefined) {
            this.messages = JSON.parse(contents); 
        }
    } catch (err) {
        console.error(err);
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
        var lowerCasePropertyName = propertyName.toLowerCase();
        if (object[lowerCasePropertyName] === undefined && addIfUndefined !== undefined) {
            object[lowerCasePropertyName] = addIfUndefined;
        }
        result = object[lowerCasePropertyName];
    }
    return result;
}

/**
 * Adds a Message to a tree message structure
 * @param {object} messageTree hirachical structure to store all messages of a floor
 * @param {object} message message to add
 * @param {string} subtreeName name of the property to select the subtree
 * @returns {object} subtree with message name
 */
MessageList.prototype.getOrAddMessageSubtree = function(messageTree, message, subtreeName) {
    var subTree = this.getFromObject(messageTree, message[subtreeName], {});
    message[subtreeName] = undefined;
    return subTree;
}


/**
 * Adds a Message to a floor message structure
 * @param {object} floorMessages hirachical structure to store all messages of a floor
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToFloor = function(floorMessages, message) {
    var roomMessages = this.getFromObject(floorMessages, message.room, {});
    message.room = undefined;
    this.addMessageToRoom(roomMessages, message);
}

/**
 * Adds a Message to a room messages structure
 * @param {object} roomMessages hirachical structure to store all messages of a room
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToRoom = function(roomMessages, message) {
    var areaMessages = this.getFromObject(roomMessages, message.area, {});
    message.area = undefined;
    this.addMessageToArea(areaMessages, message);
}

/**
 * Adds a Message to an area messages structure
 * @param {object} areaMessages hirachical structure to store all messages of an area
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToArea = function(areaMessages, message) {
    var deviceMessages = this.getFromObject(areaMessages, message.interfaceName, {});
    message.interfaceName = undefined;
    this.addMessageToDevice(deviceMessages, message);
}

/**
 * Adds a Message to a device messages structure
 * @param {object} deviceMessages hirachical structure to store all messages of a device
 * @param {object} message message to add
 */
MessageList.prototype.addMessageToDevice = function(deviceMessages, message) {
    var sensorMessages = this.getFromObject(deviceMessages, message.name, {"current":undefined, "history": []});
    message.name = undefined;
    this.addMessageToSensor(sensorMessages, message);
}

/**
 * Update the statistic of a message tree node
 * @param {object} sensorMessages hirachical structure to store all messages of a sensor
 * @param {object} message message used to update the statistic
 */
MessageList.prototype.indentifyUnexpectdChange = function(sensorMessages) {
// Todo:
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
        sensorMessages.command = message;
    } else {
        sensorMessages.current = message;
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

    if (this.statistic.received.totalMessages % 10 === 0) {
        this.saveTree();
    }

}

/**
 * Adds a message a hirachical data store
 * @param {RS485BinaryMessage} message message to add
 * @returns undefined
 */
MessageList.prototype.addMessage = function(message) {
    //var address = message.send ? message.receiver : message.sender;
    var floorMessages = this.getFromObject(this.messages, message.floor, {});
    message.floor = undefined;
    this.addMessageToFloor(floorMessages, message);
};

/**
 * Gets the message statistics as HTTPMessage
 * @param {object} res HTTP result object
 * @returns undefined
 */
MessageList.prototype.getStatisticsAsHTTPMessage = function (res) {
    res.writeHead(200, {'Content-Type': 'application/json'});
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
    var floorMessages = this.getFromObject(this.messages, floor);
    var roomMessages = this.getFromObject(floorMessages, room);
    if (roomMessages != undefined) {
        result = roomMessages;
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
    var result = {};
    var roomMessages = this.getRoomData(floor, room);
    var areaMessages = this.getFromObject(roomMessages, area);
    if (areaMessages != undefined) {
        result = areaMessages;
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
    var result = {};
    var areaMessages = this.getAreaData(floor, room, area);
    var deviceMessages = this.getFromObject(areaMessages, device);
    if (deviceMessages !== undefined) {
        result = deviceMessages;
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
    var result = {};
    var deviceMessages = this.getDeviceData(floor, room, area, device);
    var interfaceMessages = this.getFromObject(deviceMessages, interfaceProperty);
    if (interfaceMessages !== undefined) {
        result = interfaceMessages;
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
    var floorMessages = this.getFromObject(this.messages, floor);
    if (floorMessages !== undefined) {
        for (var room in floorMessages) {
            roomLinks.push(baseLink + floor + "/" + room);
        }
    }
    return roomLinks;
}


module.exports =  MessageList;