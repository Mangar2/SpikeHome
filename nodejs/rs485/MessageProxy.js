/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      MessageProxy.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

var http = require('http');
const initialPath = "/publish";

function MessageProxy (brokerPort) {
    this.brokerPort = brokerPort;
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
 * Updates a statistic about messages
 * @param {RS485BinaryMessage} message message to include in the statistic
 * @returns undefined
 */
MessageProxy.prototype.updateMessageStatistic = function(message) {
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
 * Escapes a path string
 * @param {string} path 
 * @returns {string} escaped path
 */
function escapePath(path) {
    var result = path.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); 
    result = result.replace(/ /g, '%20');
    return result;
}

/**
 * Creates a path from a message object
 * @param {object} message 
 * @returns {string} path
 */
function messageToPath(message) {
    var path = initialPath + "/" + message.floor + "/" + message.room;
    if (message.area !== undefined) {
        path += "/" + message.area;
    }
    if (message.interfaceName !== undefined) {
        path += "/" + message.interfaceName;
    }
    if (message.name !== undefined) {
        path += "/" + message.name;
    }
    path = escapePath(path);
    return path;
}

/**
 * Adds a message a hirachical data store
 * @param {RS485BinaryMessage} message message to add
 * @returns undefined
 */
MessageProxy.prototype.addMessage = function(message) {
    var putData = JSON.stringify({
        key: message.key,
        value: message.value,
        sender: message.sender,
        receiver: message.receiver,
        reason: [{
            message: "received from arduino",
            timestamp: message.timestamp
        }]
    });
    var path = messageToPath(message);
    var put_options = {
        host: '127.0.0.1',
        port: this.brokerPort,
        path: path,
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json',
            'Content-Length': putData.length
        },
        form: putData
    };

    if (this.brokerPort !== undefined) {
        var putReq = http.request(put_options, function(res) {
            res.setEncoding('utf8');
            //console.log(res);
            res.on('data', function (chunk) {
            });
        });

        putReq.write(putData);
        putReq.end();

        putReq.on('error', function(err) {
            console.error(err);
        });
    }
};

/**
 * Gets the message statistics as HTTPMessage
 * @param {object} res HTTP result object
 * @returns undefined
 */
MessageProxy.prototype.getStatisticsAsHTTPMessage = function (res) {
    res.writeHead(200, {'Content-Type': 'application/json'});
    var payload = this.statistic;
    res.end(JSON.stringify(payload));
}

module.exports =  MessageProxy;