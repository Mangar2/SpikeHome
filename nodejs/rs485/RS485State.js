/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      RS485State.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

var RS485Message = require('./RS485BinaryMessage');


var RS485State = function(address) {
    this.STATE_UNKNOWN            = 0;
    this.NEIGHBOUR_UNKNOWN        = 255;
    this.STATE_REBOOT             = 1;
    this.STATE_SINGLE             = 2;
    this.STATE_UNREGISTERED       = 3;
    this.STATE_REGISTERED         = 4;
    this.BROADCAST                = 0;

    this.MAX_WAIT_TIMER           = 100;
    this.TIMER_SMALL_PERIOD       = 3;
    this.TIMER_LARGE_PERIOD       = 7;
    this.TIMER_LOOP               = this.TIMER_SMALL_PERIOD + this.TIMER_LARGE_PERIOD;
    this.TIMEOUT_NO_ENABLE_SEND   = 4 * this.TIMER_LOOP;

    this.TOKEN                        = '!';
    this.STATE_UNCHANGED              = 0;
    this.ENABLE_SEND                  = 1;
    this.REGISTRATION_INFO            = 2;
    this.REGISTRATION_REQUEST         = 3;
    this.STATE_CHANGED                = 4;
    this.LOOP_TIMEOUT                 = 10;
    this.LOOP_START                   = 11;
    this.LOOP_SHORT_BREAK             = 12;
    this.LOOP_LONG_BREAK              = 13;

    this.state = this.STATE_UNKNOWN;
    this.timer = 0;
    this.lastEnableSend = 0;
    this.neighbour = this.NEIGHBOUR_UNKNOWN;
    this.leftmostCeibling = 0;
    this.maySend = false;
    this.myAddress = address;
    this.chain = [{address: address, version: 1}];
};

/**
 * Sets the value friendly name entry of a message. A friendly name is easier to read that the internal key
 * @param {RS485BinaryMessage} message message to be changed
 * @returns changed message (including friendly names)
 */
RS485State.prototype.setFriendlyName = function(message) {
    if (message.key === this.TOKEN) {
        switch (message.value) {
            case this.ENABLE_SEND: message.valueFriendlyName = "Enable Send"; break;
            case this.REGISTRATION_INFO: message.valueFriendlyName = "Registration Info"; break;
            case this.REGISTRATION_REQUEST: message.valueFriendlyName = "Registration Request"; break;
            default: message.valueFriendlyName = "Error, undefined value: " + message.value; break;
        }
    }
    return message;
}

/**
 * Checks, if the current state is "Registered"
 * @returns {boolean} true, if the current state is "Registered"
 */
RS485State.prototype.isRegistered = function() {
    return this.state === this.STATE_REGISTERED;
}

/**
 * returns a string representing the current state
 * @returns {string} current state
 */
RS485State.prototype.getStateString = function() {
    var result = "Undefined";
    switch (this.state) {
        case this.STATE_UNKNOWN: result = "Unknown"; break;
        case this.STATE_REBOOT: result = "Reboot"; break;
        case this.STATE_SINGLE: result = "Single"; break;
        case this.STATE_UNREGISTERED: result = "Unregistered"; break;
        case this.STATE_REGISTERED: result = "Registered"; break;
    }
    return result;
}

/**
 * Logs the current state to the console
 */
RS485State.prototype.logState = function() {
    console.log("State: " + this.getStateString());
};

RS485State.prototype.getInfo = function() {
    return {
        state: this.getStateString(),
        leftmostCeibling: this.leftmostCeibling,
        neighbour: this.neighbour,
        myAddress: this.myAddress,
        chain: this.chain
    }
}

/**
 * Creates a message signaling a state
 * @param {number} stateValue value of the state to signal
 * @returns a state signaling message fitting to the stateValue
 */
RS485State.prototype.createStateSignalingMessage = function(stateValue) {
    var message;
    if (stateValue !== this.STATE_UNKNOWN && stateValue !== this.STATE_CHANGED) {
        message = new RS485Message();
        message.setKeyValue(this.TOKEN, stateValue);
        message.send = true;
        if (stateValue === this.ENABLE_SEND) {
            message.setAddresses(this.myAddress, this.getReceiverAddress(), 0);
        } else {
            message.setAddresses(this.myAddress, this.BROADCAST, 0);
        }
        this.setFriendlyName(message);
    }
    return message;
};

/**
 * Handles a new message and adapts the status
 * @param message a message received from an RS485 communication
 * @returns a state signaling message fitting to the stateValue
 */
RS485State.prototype.processIncomingStateMessage = function(message) {
    var stateSignalingMessage;
    if (message.key === this.TOKEN && !message.hasError()) {
        var isForMe = message.isForMe(this.myAddress);
        this.updateSenderAddressChain(message.sender);
        var stateValue = this.updateState(message.value, !isForMe);
        stateSignalingMessage = this.createStateSignalingMessage(stateValue);
        if (stateSignalingMessage !== undefined) {
            stateSignalingMessage.setVersionAndLength(message.version);
        }
    }
    return stateSignalingMessage;
};


/**
 * Changes the maySend flag
 * @param {bool} maySend
 * @returns {undefined}
 */
RS485State.prototype.setMaySend = function(maySend) {
    this.maySend = maySend;
};

/**
 * Gets the address of the next receiver in the node chain
 */
RS485State.prototype.getReceiverAddress = function() {
    if (this.neighbour === this.NEIGHBOUR_UNKNOWN) {
        if (this.leftmostCeibling === this.NEIGHBOUR_UNKNOWN) {
            return this.BROADCAST;
        }
        return this.leftmostCeibling;
    } else {
        return this.neighbour;
    }
};

/**
 * Update the sender address chain, if the senderAddress is closer than current rightmost and leftmost ceibling
 * @param {number} senderAddress address of the sender of the last message
 * @returns {undefined}
 */
RS485State.prototype.updateSenderAddressChain = function(senderAddress) {
    RS485State.prototype.updateSenderAddressChain = function(senderAddress) {
        var rightCeibling =  (this.myAddress < senderAddress) && (this.neighbour > senderAddress);
        if (this.chain[this.chain.length - 1].address < senderAddress) {
            this.chain.push({address: senderAddress});
        } else {
            for (var pos = 0; pos < this.chain.length; pos++) {
                if (this.chain[pos].address == senderAddress) {
                    break;
                }
                if (this.chain[pos].address > senderAddress) {
                    this.chain.splice(pos, 0, {address: senderAddress});
                    break;
                }
            }
        }
        if (rightCeibling) {
            this.neighbour = senderAddress;
        }
        this.leftmostCeibling = Math.min(this.leftmostCeibling, senderAddress);
    };
};


/**
 * Updates the current state 
 * @param {number} request state request value
 * @param {boolean} notForMe true, if the last message was not addressed to me
 * @returns {number} new state value
 */
RS485State.prototype.updateState = function(request, notForMe) {
    var res = this.STATE_UNCHANGED;
    switch (this.state) {
        case this.STATE_UNKNOWN: res = this.processUnknown(request, notForMe); break;
        case this.STATE_REBOOT: res = this.processReboot(request, notForMe); break;
        case this.STATE_SINGLE: res = this.processSingle(request, notForMe); break;
        case this.STATE_UNREGISTERED: res = this.processUnregistered(request, notForMe); break;
        case this.STATE_REGISTERED: res = this.processRegistered(request, notForMe); break;
    }
    return res;
};

/**
 * Calculates a new state and updates it without having a new message. 
 * @returns undefined or a message that must then be sent on a new state. 
 */
RS485State.prototype.updateStateNoMessage = function() {
    var loopState,
        res = this.STATE_UNCHANGED;
    if (this.timer >= this.MAX_WAIT_TIMER) {
        res = this.updateState(this.LOOP_TIMEOUT);
    } else {
        loopState = this.timer % this.TIMER_LOOP;
        if (loopState === 0) {
            res = this.updateState(this.LOOP_START);
        } else if (loopState === this.TIMER_SMALL_PERIOD) {
            res = this.updateState(this.LOOP_SHORT_BREAK);
        } else if (loopState === this.TIMER_LARGE_PERIOD) {
            res = this.updateState(this.LOOP_LONG_BREAK);
        }
    }
    if (res !== this.STATE_CHANGED) {
        this.timer += 1;
    }
    return this.createStateSignalingMessage(res);
};


/**
 * Sets a new state
 */
RS485State.prototype.setState = function(newState) {
    this.timer = 0;
    this.state = newState;
    this.logState();
};

/**
 * Calculates an enable send request 
 * It is either "enable send", if it is directly addressed to myself or a registration request, if it is boradcasted
 * @returns enable send request
 */
RS485State.prototype.calculateEnableSend = function() {
    var res = 0;
    if (this.getReceiverAddress() === this.BROADCAST) {
        res = this.REGISTRATION_REQUEST;
    } else {
        res = this.ENABLE_SEND;
    }
    return res;
};

/**
 * processes an enable send command (token) while in any other state than registered.
 * If the command is not for myself, we set the state to unregistered as we know now that there are other devices on the bus
 * If the command is for myself we set the state to registered, as we know that at least one device added us to this
 * device chain and will continue to send us tokens
 */
RS485State.prototype.processEnableSendWhenNotRegistered = function(notForMe) {
    if (notForMe) {
        this.setState(this.STATE_UNREGISTERED);
    } else {
        this.setState(this.STATE_REGISTERED);
        this.maySend = true;
    }
    return this.STATE_CHANGED;
};

/**
 * Processes a request while in "unknown" state
 * @param {number} request request number
 * @param {boolean} notForMe true, if the request is not addressed to myself
 * @returns {number} state change information
 */
RS485State.prototype.processUnknown = function(request, notForMe) {
    var res = this.STATE_UNCHANGED;
    this.setMaySend(false);
    switch (request) {
    case this.ENABLE_SEND:
        res = this.processEnableSendWhenNotRegistered(notForMe);
        break;
    case this.REGISTRATION_INFO:
        // Only registration request will lead to a registration
        break;
    case this.REGISTRATION_REQUEST:
        this.setState(this.STATE_UNREGISTERED);
        res = this.REGISTRATION_INFO;
        break;
    case this.LOOP_START:
        if (this.timer === 0) {
            this.neighbour = this.NEIGHBOUR_UNKNOWN;
            this.leftmostCeibling = this.NEIGHBOUR_UNKNOWN;
        }
        break;
    case this.LOOP_TIMEOUT:
        this.setState(this.STATE_REBOOT);
        res = this.STATE_CHANGED;
        break;
    }
    return res;
};

/**
 * Processes a request while in "reboot" state. In the reboot state everything is unknown.
 * We are only listen for registration requests.
 * @param {number} request request number
 * @param {boolean} notForMe true, if the request is not addressed to myself
 * @returns {number} state change information
 */
RS485State.prototype.processReboot = function(request, notForMe) {
    var res = this.STATE_UNCHANGED;
    this.maySend = false;
    switch (request) {
    case this.ENABLE_SEND:
        res = this.processEnableSendWhenNotRegistered(notForMe);
        break;
    case this.REGISTRATION_INFO:
        // Only a registration request will lead to a registration
        break;
    case this.REGISTRATION_REQUEST:
        this.setState(this.STATE_UNREGISTERED);
        res = this.REGISTRATION_INFO;
        break;
    case this.LOOP_START:
        res = this.calculateEnableSend();
        break;
    case this.LOOP_TIMEOUT:
        this.setState(this.STATE_SINGLE);
        res = this.STATE_CHANGED;
        break;
    }
    return res;
};

/**
 * Processes a request while in "single" state. 
 * In single state, we did not detect any other sending device on the RS485 bus. We are waiting for new
 * devices joining the bus and are sending data.  
 * @param {number} request request number
 * @param {boolean} notForMe true, if the request is not addressed to myself
 * @returns {number} state change information
 */
RS485State.prototype.processSingle = function(request, notForMe) {
    var res = this.STATE_UNCHANGED;

    switch (request) {
    case this.ENABLE_SEND:
        res = this.processEnableSendWhenNotRegistered(notForMe);
        break;
    case this.REGISTRATION_INFO:
        this.setMaySend(false);
        this.setState(this.STATE_UNKNOWN);
        res = this.STATE_CHANGED;
        break;
    case this.REGISTRATION_REQUEST:
        this.setMaySend(false);
        this.setState(this.STATE_UNREGISTERED);
        res = this.REGISTRATION_INFO;
        break;
    case this.LOOP_START:
        this.setMaySend(false);
        res = this.REGISTRATION_REQUEST;
        break;
    case this.LOOP_SHORT_BREAK:
        this.setMaySend(true);
        break;
    case this.LOOP_TIMEOUT:
        this.timer = 0;
        break;
    }
    return res;
};

/**
 * Processes a request while in "unregistered" state. 
 * In unregistered state, we already detectd other devices on the bus, but we did not yet register to the bus.
 * We are sending registration informations and requests for other devices to register.  
 * @param {number} request request number
 * @param {boolean} notForMe true, if the request is not addressed to myself
 * @returns {number} state change information
 */
RS485State.prototype.processUnregistered = function(request, notForMe) {
    var res = this.STATE_UNCHANGED;
    this.setMaySend(false);
    switch (request) {
    case this.ENABLE_SEND:
        if (!notForMe) {
            this.setState(this.STATE_REGISTERED);
            this.setMaySend(true);
        }
        res = this.STATE_CHANGED;
        break;
    case this.REGISTRATION_INFO:
        break;
    case this.REGISTRATION_REQUEST:
        res = this.REGISTRATION_INFO;
        break;
    case this.LOOP_TIMEOUT:
        this.setState(this.STATE_UNKNOWN);
        res = this.STATE_CHANGED;
        break;
    }
    return res;
};

/**
 * Sub function to process a registered state. Things we do, in the first part of a standard loop:
 * Checking for lost tokens, deactivating may send, requesting registrations of devices ...
  * @param {number} request request number
 * @param {boolean} notForMe true, if the request is not addressed to myself
 * @returns {number} state change information
 */
RS485State.prototype.registeredShortLoopBreak = function() {
    var res = this.STATE_UNCHANGED;
    this.tokenLost = (this.lastEnableSend + this.TIMEOUT_NO_ENABLE_SEND <= this.timer);
    if (this.timer === this.TIMER_SMALL_PERIOD || this.tokenLost) {
        this.lastEnableSend = this.timer;
        this.setMaySend(false);
        if (this.neighbour === this.NEIGHBOUR_UNKNOWN && !this.tokenLost) {
            res = this.REGISTRATION_REQUEST;
        } else {
            res = this.ENABLE_SEND;
        }
    }
    return res;
};

/**
 * Processes a request while in "registered" state. This is the normal state after the device registered to the bus.
 * In registered state we send messages, listen for messages, requests for new devices to register and are checking
 * for timeout situations
 * @param {number} request request number
 * @param {boolean} notForMe true, if the request is not addressed to myself
 * @returns {number} state change information
 */
RS485State.prototype.processRegistered = function(request, notForMe) {
    var res = this.STATE_UNCHANGED;
    switch (request) {
    case this.ENABLE_SEND:
        if (!notForMe) {
            this.setMaySend(true);
            this.timer = 0;
        } else {
            this.setMaySend(false);
            this.lastEnableSend = this.timer;
        }
    case this.REGISTRATION_INFO: break;
    case this.REGISTRATION_REQUEST: break;
    case this.LOOP_SHORT_BREAK:
        res = this.registeredShortLoopBreak();
        break;
    case this.LOOP_LONG_BREAK:
        if (this.timer === this.TIMER_LARGE_PERIOD && this.neighbour === this.NEIGHBOUR_UNKNOWN && this.leftmostCeibling !== this.NEIGHBOUR_UNKNOWN) {
            res = this.ENABLE_SEND;
        }
        break;
    case this.LOOP_TIMEOUT:
        this.setState(this.STATE_UNREGISTERED);
        res = this.STATE_CHANGED;
        break;
    }
    return res;
};

module.exports = RS485State;