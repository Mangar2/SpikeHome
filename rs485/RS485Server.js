/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485Server.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

const sensorConfig = require('./config.json');

var RS484Message = require('./RS485BinaryMessage');
var RS485IO = require('./RS485IO');
var querystring = require('querystring');
const SUPPORTED_MESSAGE_VERSION = 1;


var serialIO = new RS485IO(SUPPORTED_MESSAGE_VERSION, sensorConfig.addresses, sensorConfig.interfaces);

var RS485Server = function(RS485Address, serialCOMPort, baudrate, httpPort, sqlPort) {
    this.http = require('http');
    this.message = new RS484Message;
    this.openArduinoPort(serialCOMPort, baudrate);
    this.address = RS485Address;
    this.httpPort = httpPort;
    this.sqlPort = sqlPort;
    serialIO.setAddress(this.address);
};

/**
 * Call to close the serial port
 */
RS485Server.prototype.close = function () {
    serialIO.close();
}

/**
 * Call to close the serial port
 */
RS485Server.prototype.openArduinoPort = function (serialCOMPort, baudrate) {
    var that = this;
    serialIO.openArduinoPort(serialCOMPort, baudrate, function() {
        that.createHttpServer(that.httpPort);

        serialIO.serialPort.on('data', function (stream) {
            try {
                while (that.readAndStoreMessages(stream)) {};
            }
            catch (err) {
                console.log(err);
            }
        });
    });
}


/**
 * Reads messages from a serial stream and stores it to a message tree
 * @param that pointer to the class object
 * @param {array} stream array of received bytes
 */
RS485Server.prototype.readAndStoreMessages = function (stream) {
    var that = this;
    serialIO.readAndStoreMessages(stream, function(message) {
        that.storeToSQL(message);
    }); 
   
};

/**
 * Informes others about a message received
 * @param {RS485BinaryMessage} receivedMessage received message
 * @returns undefined
 */
RS485Server.prototype.storeToSQL = function (receivedMessage) {
    var putData = querystring.stringify({
        key: receivedMessage.key,
        value: receivedMessage.value,
        address: receivedMessage.sender,
        timestamp: receivedMessage.timestamp
    });

    var put_options = {
        host: '127.0.0.1',
        port: this.sqlPort,
        path: '/update',
        method: 'PUT',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
            'Content-Length': putData.length
        },
        form: putData
    };

    if (this.sqlPort !== undefined) {

        var putReq = this.http.request(put_options, function(res) {
            res.setEncoding('utf8');
            res.on('data', function (chunk) {
                console.log('Response: ' + chunk);
            });
        });

        putReq.write(putData);
        putReq.end();

        putReq.on('error', function(err) {
            //console.error(err);
        });
    }
}



/**
 * Removes all history properties of payload
 * @param {object} payload object to return
 * @returns clone of payload without history
 */
RS485Server.prototype.removeHistory = function(payload) {
    var result;    
    if (typeof payload === "object") {
        result = {};
        for (const key in payload) {
            if (key !== "history" && payload.hasOwnProperty(key)) {
                var property = payload[key];
                result[key] = this.removeHistory(property);
            }
        }
    } else {
        result = payload;
    }
    return result;
}

RS485Server.prototype.sensorInterface = function (req, path, res) {
    var floor = path[1];
    var room = path[2];
    var area = path[3];
    var device = path[4];
    var property = path[5];
    var payload = "";
    var addHistory = req.headers.history;
    if (floor === undefined || floor === "") {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        payload = serialIO.messageList.getFloorLinks("/sensor/");
    } else if (room === undefined || room === "") {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        payload = serialIO.messageList.getRoomLinks("/sensor/", floor);
    } else if (area === undefined || area === "") {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        payload = serialIO.messageList.getRoomData(floor, room);
    } else if (device === undefined || device === "") {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        payload = serialIO.messageList.getAreaData(floor, room, area);
    } else  if (property === undefined || property === "") {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        payload = serialIO.messageList.getDeviceData(floor, room, area, device);
    } else {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        payload = serialIO.messageList.getPropertyData(floor, room, area, device, property);
    }
    if (addHistory !== 'true' && addHistory !== '1') {
        payload = this.removeHistory(payload);
    }

    res.end(JSON.stringify(payload));
}

/**
 * Handels all get requests to the interface
 */
RS485Server.prototype.onGet = function(req, path, res) {
    var interfaceName = path[0];
    if (interfaceName === "sensor") {
        this.sensorInterface(req, path, res);
    } else if (interfaceName === "info") {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        var payload = serialIO.state.getInfo();
        res.end(JSON.stringify(payload));
    } else if (interfaceName === "statistic") {
        serialIO.messageList.getStatisticsAsHTTPMessage(res);
    } else if (interfaceName === "trace") {
        serialIO.trace.getTraceAsHTTPMessage(path[1], res);
    }
};

/**
 * Sends a message
 * @param {number} targetAddress target address of the message
 * @param {string} key single char key
 * @param {number} value new value
 * @param {reason} object reason for the message
 */
RS485Server.prototype.sendMessage = function(targetAddress, key, value, reason) {
    var message = new RS484Message();
    reason = [reason].push({action: "http put", timestamp: new Date().toISOString()});
    message.setAddresses(this.address, targetAddress, 1);
    message.setKeyValue(key, value);
    message.setReason(reason);
    message.timestamp = new Date().toISOString();
    message.send = true;
    serialIO.addBinaryMessageToMessageTree(message);
    serialIO.addMessageToSendQueue(message);
}

/**
 * Handles all put requests to the interface
 * @param {object} message message containing location and interface of th sensor
 * @param {array} body path in array form 
 * @param {object} res http result structure
 */
RS485Server.prototype.setSensorValue = function(message, res) {
    try {
        serialIO.addMessageToMessageTree(message);

        res.writeHead(204, {'Content-Type': 'text/plain'});
        res.end("");

    } catch (error) {
        res.writeHead(400, {'Content-Type': 'text/plain'});
        console.log(error);
        res.end(error);
    }
}

/**
 * Handles all put requests to the interface
 * @param {object} message message containing location and interface of th sensor
 * @param {array} body path in array form 
 * @param {object} res http result structure
 */
RS485Server.prototype.setSensor = function(message, res) {
    try {
        var info = serialIO.getAddressAndKey(message);
        
        that.sendMessage(info.address, info.key, message.value, message.reason);

        res.writeHead(204, {'Content-Type': 'text/plain'});
        res.end("");

    } catch (error) {
        res.writeHead(400, {'Content-Type': 'text/plain'});
        console.log(error);
        res.end(error);
    }
}

/**
 * Handles all put requests to the interface
 * @param {object} req object containing the request
 * @param {array} path path in array form 
 * @param {object} res http result structure
 */
RS485Server.prototype.onPut = function(req, path, res) {
    var body = '';
    var httpInterface = path[0];
    var message = {
        floor: path[1],
        room: path[2],
        area: path[3],
        interfaceName: path[4],
        name: path[5]
    }
    var that = this;

    req.on('data', function(chunk) {
        //console.log(chunk);
        body += chunk;
    });
    req.on('end', function() {
        try {
            var parsedBody = querystring.parse(body);
            
            if (parsedBody.value !== undefined) {
                message.value = parseInt(parsedBody.value, 10);
                if (parsedBody.reason !== undefined) {
                    message.reason = JSON.parse(parsedBody.reason);
                }
                if (parsedBody.timestamp !== undefined) {
                    message.timestamp = JSON.parse(parsedBody.timestamp);
                } else {
                    message.timestamp = new Date().toISOString();
                }
                if (httpInterface === "sensor") {
                    that.setSensor(message, res);
                } else if (httpInterface === "sensorValue") {
                    that.setSensorValue(message, res);
                } else {
                    res.writeHead(400, {'Content-Type': 'text/plain'});
                    res.end("Illegal interface");
                }
            }
        }
        catch (err) {
            console.log(body);
            console.log(err);
        }
    });
}

/**
 * Old interface sending a key/value pair to an address
 * @param {object} req object containing the request
 * @param {array} path path in array form 
 * @param {object} res http result structure
 */ 
RS485Server.prototype.onPost = function(req, path, res) {
    var httpInterface = path[0];
    var body = "";
    var that = this;
    if (httpInterface === "set") {    
        req.on('data', function(chunk) {
            //console.log(chunk);
            body += chunk;
        });
        req.on('end', function() {
            var parsedBody = querystring.parse(body);
            that.sendMessage(parsedBody.address, parsedBody.id, parsedBody.value, ["post command"]);
            res.writeHead(204, {'Content-Type': 'text/plain'});
            res.end();
    
        });
    }
}

/**
 * Creates a http server listening
 * @param {number} port port to listen to
 */
RS485Server.prototype.createHttpServer = function(port) {
    var that = this;
    // Create server to listen for intranet calls
    this.http.createServer(function (req, res) {
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
            if (method === "post") {
                that.onPost(req, path, res);
            }
        } catch (err) {
            console.log(err);
        }
    }).listen(port, function () {
        console.log("Intranet server listening on: http://localhost:%s", port);
    });

}
    
module.exports = RS485Server;