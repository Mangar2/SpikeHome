/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      HTTPServer.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

const MessageTree = require('./MessageTree');
const http = require('http');



var HTTPServer = function(dataDirectory, serviceName) {
    this.messageTree = new MessageTree(dataDirectory, serviceName);

    this.isEmptyString = function(string) {
        return string === undefined || string === "";
    }
};

/**
 * Removes all history properties of payload
 * @param {object} payload object to return
 * @returns clone of payload without history
 */
function removeHistory(payload) {
    var result;     
    if (typeof payload === "object") {
        try {
            result = {};
            for (const key in payload) {
                if (key !== "history") {
                    var property = payload[key];
                    result[key] = removeHistory(property);
                }
            }
        } catch (err) {
            console.log(err);
            console.error(payload);
        }
    } else {
        result = payload;
    }
    return result;
}

/**
 * Reads the value from a parsed body (if available)
 * @param {object} parsedBody parsed Message Body 
 * @returns {object} the message value
 */
function getValue(parsedBody) {
    var value;
    if (parsedBody !== undefined && parsedBody.value !== undefined) {
        try {
            value = parseFloat(parsedBody.value);
        } catch (error) {
            value = undefined;
        }
    } 

    return value;
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
 * Reads the reason object from a parsed body (if available)
 * @param {object} parsedBody parsed Message Body 
 * @returns {object} the message reason
 */
function getReason(parsedBody) {
    var reason = [];
    if (parsedBody !== undefined && parsedBody.reason !== undefined) {
        reason = parsedBody.reason;
        if (!Array.isArray(reason)) {
            reason = [reason];
        }
    } 
    
    reason.push({
        message: "published to broker",
        timestamp: getLocalTimeAsISOString()
    });

    return reason;
}


/**
 * Creates a message from a array converted http path and the parsed body
 * @param {object} parsedBody parsed body (as object)
 * @returns {object} message containing the path and the body elements
 */
HTTPServer.prototype.createMessage = function(parsedBody) {
    var message = {};
    if (parsedBody !== undefined) {
        message = parsedBody;
        message.value = getValue(parsedBody);
        message.reason = getReason(parsedBody);
    };

    return message;
}


/**
 * Implements the sensor interface, returning sensor information based on the path 
 * @param {object} req HTTP request object
 * @param {array} path HTTP path spliced into an array
 * @param {obect} res HTTP result structure
 */
HTTPServer.prototype.sensorInterface = function (req, path, res) {
    var payload = "";
    var addHistory = req.headers.history;
    res.writeHead(200, {'Content-Type': 'application/json'});
    payload = this.messageTree.getMessages(path);
    if (payload.payload !== undefined && addHistory !== 'true' && addHistory !== '1') {
        payload = removeHistory(payload.payload);
    }
    res.end(JSON.stringify(payload));
}

/**
 * Handels all get requests to the interface
 */
HTTPServer.prototype.onGet = function(req, path, res) {
    var interfaceName = path[0];
    if (interfaceName === "sensor") {
        this.sensorInterface(req, path, res);
    } else {
        res.writeHead(400, {'Content-Type': 'text/plain'});
        res.end("Illegal interface name: " + interfaceName);
    }
};

/**
 * Publishes a message
 * @param {array} topic topic string (path) sliced to an array
 * @param {object} message message containing location and interface of th sensor
 * @param {array} body path in array form 
 * @param {object} res http result structure
 */
HTTPServer.prototype.publish = function(topic, message, res) {
    try {
        this.messageTree.addMessage(topic, message);

        res.writeHead(204, {'Content-Type': 'text/plain'});
        res.end("");

    } catch (error) {
        res.writeHead(400, {'Content-Type': 'text/plain'});
        console.error(error);
        res.end(error);
    }
}

/**
 * Handles all put requests to the interface
 * @param {object} req object containing the request
 * @param {array} path path in array form 
 * @param {object} res http result structure
 */
HTTPServer.prototype.onPut = function(req, path, res) {
    var body = '';
    var httpInterface = path[0];
    var message;
    var that = this;

    req.on('data', function(chunk) {
        body += chunk;
    });
    req.on('end', function() {
        try {
            var parsedBody = JSON.parse(body);
            if (parsedBody.value !== undefined) {
                if (httpInterface === "publish") {
                    message = that.createMessage(parsedBody);
                    that.publish(path, message, res);
                } else {
                    res.writeHead(400, {'Content-Type': 'text/plain'});
                    res.end("Illegal interface " + httpInterface);
                }
            }
        }
        catch (err) {
            console.error(body);
            console.error(err);
        }
    });
}

/**
 * Creates a http server listening
 * @param {number} port port to listen to
 */
HTTPServer.prototype.createHttpServer = function(port) {
    var that = this;
    // Create server to listen for intranet calls
    http.createServer(function (req, res) {
        try {
            var url        = require('url').parse(req.url, true);
            var method     = req.method.toLowerCase();
            var decodedUri = decodeURI(url.pathname);
            var path = decodedUri.split("/").slice(1);
            if (method === "get") {
                that.onGet(req, path, res);
            }
            if (method === "put") {
                that.onPut(req, path, res);
            }
        } catch (err) {
            console.log(err);
        }
    }).listen(port, function () {
        console.log("Intranet server listening on: http://localhost:%s", port);
    });

}
    
module.exports = HTTPServer;