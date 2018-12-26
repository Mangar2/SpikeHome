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
const { parse } = require('querystring');
const SUPPORTED_MESSAGE_VERSION = 0;


var serialIO = new RS485IO(SUPPORTED_MESSAGE_VERSION);

var RS485Server = function(RS485Address, serialCOMPort, baudrate) {
    this.http = require('http');
    this.message = new RS484Message;
    this.initMessageList(sensorConfig.addresses, sensorConfig.interfaces);
    this.openArduinoPort(serialCOMPort, baudrate);
    this.address = RS485Address;
    serialIO.setAddress(this.address);
};

/**
 * Call to close the serial port
 */
RS485Server.prototype.close = function () {
    serialIO.close();
}


RS485Server.prototype.initMessageList = function (addresses, interfaces) {
    serialIO.initMessageList(addresses, interfaces);
}

/**
 * Opens the serial port to attach the arduino devices
 * @param {string} serialPortName name of the serial port
 */
RS485Server.prototype.openArduinoPort = function (serialPortName, baudrate) {
    var that = this;
    serialIO.openSerialPort(serialPortName, baudrate,  function () {
        console.log("openSerialPort");
        setInterval(function () {
            try {
                serialIO.processTick(serialIO.messageList); 
            } 
            catch (err) {
                console.log(err);
            }
        }, 100);

        //setInterval(function () { serialIO.sendTimeOfDayInMinutes(); }, 1000 * 60);

        serialIO.serialPort.on('data', function (stream) {
            try {
                while (that.readAndStoreMessages(stream)) {};
            }
            catch (err) {
                console.log(err);
            }
        });

        serialIO.serialPort.on('error', function(err) {
            console.log(err);
        });

    });
}

/**
 * Reads messages from a serial stream and stores it to a message tree
 * @param that pointer to the class object
 * @param {array} stream array of received bytes
 */
RS485Server.prototype.readAndStoreMessages = function (stream) {
    var startIndex = 0;
    var message = new RS484Message();

    startIndex = message.setFromByteStream(stream, startIndex);
    while (message.isComplete()) {
        message = serialIO.state.setFriendlyName(message);
        serialIO.processMessageRead(message);
        if (!message.isTokenMessage()) {
            console.log(message.hexString);
        } 
        message = new RS484Message();
        startIndex = message.setFromByteStream(stream, startIndex);
    }
    
};

RS485Server.prototype.sensorInterface = function (path, res) {
    var floor = path[1];
    var room = path[2];
    var area = path[3];
    var device = path[4];
    var property = path[5];
    var payload = "";
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
    res.end(JSON.stringify(payload));
}

/**
 * Handels all get requests to the interface
 */
RS485Server.prototype.onGet = function(path, res) {
    var interfaceName = path[0];
    if (interfaceName === "sensor") {
        this.sensorInterface(path, res);
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
 * Handles all put requests to the interface
 */
RS485Server.prototype.onPut = function(req, path, headers, res) {
    var body = '';
    var httpInterface = path[0];
    var floor = path[1];
    var room = path[2];
    var area = path[3];
    var device = path[4];
    var property = path[5];
    var that = this;
    if (httpInterface === "sensor") {
        req.on('data', function(chunk) {
            //console.log(chunk);
            body += chunk;
        });
        req.on('end', function() {
            var info = serialIO.messageList.getAddressAndKey(floor, room, area, device, property);
            var message = new RS484Message();
            var parsedBody = parse(body);
            var reason = JSON.parse(parsedBody.reason);
            reason = [reason].push({action: "http put", timestamp: new Date().toISOString()});
            message.setAddresses(that.address, info.address, 1);
            message.setKeyValue(info.key, parseInt(parsedBody.value, 10));
            message.setReason(reason);
            message.timestamp = new Date().toISOString();
            message.send = true;
            serialIO.messageList.addMessage(message);
            serialIO.addMessageToSendQueue(message);
        });
    }
}

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
                that.onGet(path, res);
            }
            if (method === "put") {
                that.onPut(req, path, req.headers, res);
            }
            res.end();
        } catch (err) {
            console.log(err);
        }
    }).listen(port, function () {
        console.log("Intranet server listening on: http://localhost:%s", port);
    });
}
    
module.exports = RS485Server;