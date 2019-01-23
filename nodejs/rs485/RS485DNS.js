/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485DNS.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

var AddressMap = require ("./AddressMap.js");
var InterfaceMap = require("./InterfaceMap.js");
//const saveGetPopertyFromObject = require("SaveGetPropertyFromObject.js");

function RS485DNS (addresses, interfaces) {
    this.addressMap = new AddressMap(addresses);
    this.interfaceMap = new InterfaceMap(interfaces);
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
RS485DNS.prototype.getAddressAndKey = function(floor, room, area, device, propertyName) {
    var address = this.addressMap.getDeviceAddress(floor, room, area);
    var key = this.interfaceMap.getKey(device, propertyName);
    return {
        address: address, 
        key: key
    }
}


/**
 * Gets location and interface properties of a sensor 
 * @param {number} address of the sensor 
 * @param {key} key key (name) of the sensor
 */
RS485DNS.prototype.getSensorInfo = function(address, key) {
    var location = this.addressMap.getLocationFromAddress(address);
    var interfaceProperty = this.interfaceMap.getPropertyFromKey(key);
    return Object.assign(location, interfaceProperty);
}

/**
 * Adds the location and interface properties of a sensor message to the message
 * @param {object} message message having address (sensor address) and key (sensor name) property
 * @returns {object} if found, location (floor, room, area) and sensor interface name and property (interfaceName, name) is added to the message
 */
RS485DNS.prototype.addSensorInfoToMessage = function(message) {
    var result;
    var address = message.send ? message.reciever : message.sender;
    if (message !== undefined) {
        result = Object.assign(message, this.getSensorInfo(address, message.key));
    }
    return result;
}

module.exports =  RS485DNS;