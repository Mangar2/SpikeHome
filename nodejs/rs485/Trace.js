/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      Trace.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

const MAX_ARRAY_LENGTH = 5000;
const ARRAY_LENGTH_HYSTERESE = 1000;

var Trace = function() {
    // Array of trace entries by tracelevel (beginning with level 0)
    this.traceEntries = [];
    // Maximal tracelevel
    this.MAX_LEVEL = 4;
    for(var index = 0; index <= this.MAX_LEVEL; index++) {
        this.traceEntries[index] = [];
    }
}

/**
 * Adds a trace entry
 * @param {number} level trace level of the entry
 * @param {object} trace trace object to add
 * @returns undefined
 */
Trace.prototype.addTrace = function(level, trace) {
    for (var i = level; i <= this.MAX_LEVEL; i++) {
        this.traceEntries[i] = [trace].concat(this.traceEntries[i]);
        if (this.traceEntries[i].length > MAX_ARRAY_LENGTH) {
            var newLength = MAX_ARRAY_LENGTH - ARRAY_LENGTH_HYSTERESE;
            if (newLength < 0) {
                newLength = 0;
            }
            this.traceEntries[i] = this.traceEntries[i].slice(MAX_ARRAY_LENGTH - ARRAY_LENGTH_HYSTERESE);
        }
    }
}

/**
 * gets a list of trace entries 
 * @param {number} level number of the trace level 
 * @returns {array} an array of trace entries
 */
Trace.prototype.getTraceAsHTTPMessage = function(level, res) {
    var payload = {};
    res.writeHead(200, {'Content-Type': 'application/json'});

    if (level <= this.MAX_LEVEL) {
        payload = this.traceEntries[level];
    }

    res.end(JSON.stringify(payload));
}

module.exports = Trace;