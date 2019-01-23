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
const ONE_MINUTE_IN_MILLISECONDS = 60 * 1000;
const MAX_HISTORY_LENGTH = 120;
const HISTORY_LENGTH_HYSTERESE = 20;
const MAX_DATA_LEVELS = 4;

function MessageTree (dataDirectory, serviceName) {
    this.messages = {};
    this.filename = serviceName;
    this.directory = dataDirectory;
    if (this.filename === undefined) {
        this.filename = "Broker";
    }
    if (this.directory === undefined) {
        this.directory = ".";
    }
    this.writeTimestamp = Date.now();
    this.readTree();

    this.isWritingFile = function() {
        return this.writeTimeStamp = undefined;
    }

}

/**
 * Gets the local time in ISO string format
 */
function getLocalTimeAsISOString() {
    var tzoffsetInMinutes = (new Date()).getTimezoneOffset() * 60000;
    var localISOTime = (new Date(Date.now() - tzoffsetInMinutes)).toISOString().slice(0, -1);
    return localISOTime;
}

/**
 * Checks, if a function is a string
 * @param {any} x 
 */
function isString(x) {
    return Object.prototype.toString.call(x) === "[object String]"
}

/**
 * Deletes old files from the data directory
 */
function deleteOldFiles(directory, filename) {
    var fileMatch = new RegExp("^" + filename);
    fs.readdir(directory, function(err, files) {
        if (err) {
            console.error(err);
        } else {
            files.sort();
            for (var index = 0; index < files.length - 5; index ++) {
                var filename = files[index];
                if (filename.match(fileMatch)) {
                    fs.unlink(directory + filename, function(err) {
                        if (err) {
                            console.error(err);
                        } else {
                            console.log("Deleted " + directory + filename);
                        }
                    });
                }
            }
        }
    });
}

/**
 * Saves the tree to a file (existing file is overwritten)
 * @returns undefined
 */
MessageTree.prototype.saveTree = function() {
    var directory = this.directory + "/";
    var dateString = getLocalTimeAsISOString();
    dateString = dateString.replace(/:/g, '');
    var filename = this.filename;
    var filepath = directory + this.filename + dateString + ".json";
    var that = this;

    if (this.isWritingFile()) {
        return;
    }
    try {
        this.writeTimestamp = undefined;
        fs.writeFile(filepath, JSON.stringify(this.messages), function(err) {
            if (err) {
                console.error(err);
            } else {
                deleteOldFiles(directory, filename);
            }
            console.log("Written " + filepath);
            that.writeTimestamp = Date.now();
        });
    } catch (err) {
        console.error(err);
        this.writeTimestamp = Date.now();
    }
}

/**
 * Reads the newest file from an array of files, beginning with the last filename in the array
 * It stops, when one file could be read successfully
 * @param {array} files array of filenames in the current directory
 */
MessageTree.prototype.readNewestFile = function(files) {
    var directory = this.directory + "/";
    var fileMatch = new RegExp("^" + this.filename);

    for (var index = files.length - 1; index >= 0; index--) {
        var filename = files[index];
        var filepath = directory + filename;
        if (filename.match(fileMatch)) {
            try {
                var contents = fs.readFileSync(filepath);
                if (contents !== undefined) {
                    console.log("Read: " + filepath);
                    this.messages = JSON.parse(contents); 
                    break;
                }
            }
            catch (err) {
                console.error(err);
            }
        }
    }
}

/**
 * Reads the message tree from a file
 * @returns undefined
 */
MessageTree.prototype.readTree = function() {
    var directory = this.directory + "/";
    var that = this;
    try {
        fs.readdir(directory, function(err, files) {
            if (err) {
                console.error(err);
            } else {
                files.sort();
                that.readNewestFile(files);
            }
        });
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
function getFromObject(object, propertyName, addIfUndefined) {
    var result;
    if (object !== undefined && propertyName !== undefined && isString(propertyName)) {
        var lowerCasePropertyName = propertyName.toLowerCase();
        if (object[lowerCasePropertyName] === undefined && addIfUndefined !== undefined) {
            object[lowerCasePropertyName] = addIfUndefined;
        }
        result = object[lowerCasePropertyName];
    }
    return result;
}


/**
 * Update the statistic of a message tree node
 * @param {object} sensorMessages hirachical structure to store all messages of a sensor
 * @param {object} message message used to update the statistic
 * @returns {string|undefined} event text based on statistic information
 */
MessageTree.prototype.updateStatistic = function(sensorMessages, message) {
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
MessageTree.prototype.addMessageToSensor = function(sensorMessages, message) {
    var history = sensorMessages.history;
    var event = this.updateStatistic(sensorMessages, message);
    sensorMessages.current = message;
    if (event !== undefined) {
        sensorMessages.current.event = event;
    }
    if (history.length === 0 || history[0].value !== message.value) {
        sensorMessages.history = [sensorMessages.current].concat(history);
        if (sensorMessages.history.length > MAX_HISTORY_LENGTH) {
            var newLength = MAX_HISTORY_LENGTH - HISTORY_LENGTH_HYSTERESE;
            if (newLength <= 0) {
                newLength = 1;
            }
            sensorMessages.history = sensorMessages.history.slice(newLength);
        }
    }
}

/**
 * Recursively searches a node in the message tree corresponding to the path
 * @param {object} pathArray array of path elements
 * @returns {object} child node to add messages to
 */
MessageTree.prototype.searchOrCreateNode = function(pathArray) {
    var childNode = this.messages;
    for (var curIndex = 1; curIndex < pathArray.length; curIndex++) {
        var property = pathArray[curIndex];
        if (childNode !== undefined && property !== undefined) {
            childNode = getFromObject(childNode, property, {});
        }
    }
    if (!childNode.hasOwnProperty("current")) {
        childNode.current = {};
    }
    if (!childNode.hasOwnProperty("history")) {
        childNode.history = [];
    }
    return childNode;
}

/**
 * Adds a message a hirachical data store
 * @param {RS485BinaryMessage} message message to add
 * @returns undefined
 */
MessageTree.prototype.addMessage = function(pathArray, message) {
    var node = this.searchOrCreateNode(pathArray);
    this.addMessageToSensor(node, message);
    if (this.writeTimestamp !== undefined && (Date.now() - this.writeTimestamp) > ONE_MINUTE_IN_MILLISECONDS) {
        this.saveTree();
    }
};


/**
 * Gets a list of links leading to all childNodes in the message tree
 * @param {string} baseLink base link to the message tree
 */
MessageTree.prototype.getLinks = function (baseLink, node) {
    var links = [];
    for (var property in node) {
        var link = {
            rel: property,
            href: baseLink + property 
        }
        links.push(link);
    }
    return links;
}

/**
 * Recursively searches a node in the message tree corresponding to the path
 * @param {object} pathArray array of path elements
 */
MessageTree.prototype.getPathAndNode = function (pathArray) {
    var path = pathArray[0];
    var childNode = this.messages;
    for (var curIndex = 1; curIndex < pathArray.length; curIndex++) {
        var property = pathArray[curIndex];
        if (childNode !== undefined) {
            childNode = getFromObject(childNode, property);
            path = path + "/" + pathArray[curIndex];
        }
    }

    return {
        path: path + "/",
        node: childNode
    }
}

/**
 * Calulates the remaining tree depth of the current node
 * @param {object} node curent node in the tree
 * @returns {number} remaining depth of the leftmost path 
 */
MessageTree.prototype.getRemainingDepth = function(node) {
    var result = 0;
    var curNode = node;
    while ((typeof curNode === 'object') && result < 10) {
        var keys = Object.keys(curNode);
        if (keys.length == 0) {
            break;
        }
        var property = keys[0];
        curNode = curNode[property];
        result++;
    }
    return result;
}


/**
 * Gets a list of links leading to all stored floors in the message tree
 * @param {object} pathArray array of path elements
 */
MessageTree.prototype.getMessages = function (pathArray) {
    var pathAndNode = this.getPathAndNode(pathArray);
    var links = this.getLinks(pathAndNode.path, pathAndNode.node);
    var remainingDepht = this.getRemainingDepth(pathAndNode.node);
    var result = {};
    //console.log(pathAndNode);
    //console.log(this.messages);

    if (remainingDepht <= MAX_DATA_LEVELS) {
        result.payload = pathAndNode.node;
    }
    result.links = links;
    return result;
}


module.exports =  MessageTree;