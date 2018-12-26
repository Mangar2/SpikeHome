/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      AddressMap.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */
'use strict';

function AddressMap (addresses) {
    this.addressToRoom = {};
    this.roomToAddress = {};

    var rooms = addresses.payload.settings;
    for (var roomIndex = 0; roomIndex < rooms.length; roomIndex++) {
        var room = rooms[roomIndex];
        var areas = room.id;
        for (var area in areas) {
            if (areas.hasOwnProperty(area)) {
                var  address = areas[area];
                this.addressToRoom[address] = { 
                    "room" : room.room,
                    "floor": room.floor,
                    "area": area
                }
                this.addEntryToRoomToAddressMap(room.floor, room.room, area, address);
            }
        }
    }
};

/**
 * Savely gets a property from an object. 
 * @param {object} object to get entry from
 * @param {index} propertyName name of the property to retrieve
 * @param {anything} addInfUndefine property to add, if there is no entry
 * @result property of the object or undefined, if the property is not retrievable
 */
AddressMap.prototype.getFromObject = function(object, propertyName, addIfUndefined) {
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
 * Adds an entry to the address lookup table
 * @param {string} floor floor of the device
 * @param {string} room room of the device
 * @param {string} area area of the device
 * @param {number} address of the device
 * @returns undefined
 */
AddressMap.prototype.addEntryToRoomToAddressMap = function(floor, room, area, address) {
    var floorMap = this.getFromObject(this.roomToAddress, floor, {});
    var roomMap = this.getFromObject(floorMap, room, {});
    if (roomMap !== undefined && area !== undefined && address !== undefined) {
        roomMap[area] = address;
    }
}

/**
 * Lookup a address of a device
 * @param {string} floor floor of the device
 * @param {string} room room of the device
 * @param {string} area area of the device
 * @returns {number} address of the device
 */
AddressMap.prototype.getDeviceAddress = function(floor, room, area) {
    var result;
    var floorMap = this.getFromObject(this.roomToAddress, floor);
    var roomMap = this.getFromObject(floorMap, room);
    if (roomMap !== undefined && area !== undefined && roomMap[area] !== undefined) {
        result = roomMap[area];
    }
    return result;
}

/**
 * Lookup an address and returns the floor, the room and the area of the location
 * @param {number} address the internal address of a sensor area
 * @returns {object} an object containing "room", "floor" and "area" matching the address
 */
AddressMap.prototype.getLocationFromAddress = function (address) {
    var result = { "room": "unknown", "floor": "unknown", "area": "main" };
    if (this.addressToRoom[address] !== undefined) {
        result = this.addressToRoom[address];
    }
    return result;
};

module.exports =  AddressMap;