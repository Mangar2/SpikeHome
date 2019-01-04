import { Module } from "module";

/**
 * Savely gets a property from an object. 
 * @param {object} object to get entry from
 * @param {index} propertyName name of the property to retrieve
 * @param {anything} addInfUndefine property to add, if there is no entry
 * @result property of the object or undefined, if the property is not retrievable
 */
module.exports = function(object, propertyName, addIfUndefined) {
    var result;
    if (object !== undefined && propertyName !== undefined) {
        if (object[propertyName] === undefined && addIfUndefined !== undefined) {
            object[propertyName] = addIfUndefined;
        }
        result = object[propertyName];
    }
    return result;
}

