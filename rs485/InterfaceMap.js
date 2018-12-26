/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      InterfaceMap.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

function InterfaceMap (interfaces) {
    this.keyToProperty = {};
    this.PropertyToKey = {};

    var settings = interfaces.payload.settings;
    for (var settingsIndex = 0; settingsIndex < settings.length; settingsIndex++) {
        var setting = settings[settingsIndex];
        var interfaceName = setting.friendlyName;
        var capabilities = setting.capabilities;
        for (var capabilityIndex = 0; capabilityIndex < capabilities.length; capabilityIndex++) {
            var capability = capabilities[capabilityIndex];
            var properties = capability.properties.supported;
            this.addPropertiesToMaps(properties, interfaceName);
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
InterfaceMap.prototype.getFromObject = function(object, propertyName, addIfUndefined) {
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
 * Adds a list of properties to the maps (keyToProperty and PropertyToKey)
 * @param {object} properties an object of properties 
 * @param {string} interfaceName name of the interface
 * @returns undefined
 */
InterfaceMap.prototype.addPropertiesToMaps = function (properties, interfaceName) {
    for (var propertyIndex = 0; propertyIndex < properties.length; propertyIndex++) {
        var property = properties[propertyIndex];
        var keyId = property.keyId;
        if (keyId !== undefined) {
            if (this.keyToProperty[keyId] !== undefined) {
                throw "double keyId " + keyID + " in " + interfaceName;
            }
            this.keyToProperty[property.keyId] = { 
                "interfaceName" : interfaceName,
                "name": property.name
            }
            this.addToPropertyToKeyMap(interfaceName, property.name, property.keyId);
        }
    }
}

/**
 * Adds a property to the map from interface, property to key
 * @param {string} interfaceName name of the interface
 * @param {string} propertyName name of the property
 * @param {char} key key assigned to the property
 * @returns undefined
 */
InterfaceMap.prototype.addToPropertyToKeyMap = function(interfaceName, propertyName, key) {
    var interfaces = this.getFromObject(this.PropertyToKey, interfaceName, {});
    if (interfaces !== undefined && propertyName !== undefined && key !== undefined) {
        interfaces[propertyName] = key;
    }
}

/**
 * Gets interface name and property name of a key
 * @param {string} key single char key 
 * @returns {object} object with interfaceName and name (propertyname) property
 */
InterfaceMap.prototype.getPropertyFromKey = function (key) {
    var result = { "interfaceName": "unknown", "name": "unknown" };
    if (this.keyToProperty[key] !== undefined) {
        result = this.keyToProperty[key];
    }
    return result;
};

/**
 * Gets a key
 * @param {string} interfaceName name of the interface
 * @param {string} propertyName name of the property
 * @returns {string} single char key (if available, else undefined)
 */
InterfaceMap.prototype.getKey = function(interfaceName, propertyName) {
    var interfaces = this.getFromObject(this.PropertyToKey, interfaceName);
    var key = this.getFromObject(interfaces, propertyName);
    return key;
}

module.exports =  InterfaceMap;