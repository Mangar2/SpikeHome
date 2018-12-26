/**
 * ---------------------------------------------------------------------------------------------------
 * This software is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version 3. It is furnished
 * "as is", without any support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * File:      rs485.js
 *
 * Author:      Volker Böhm
 * Copyright:   Volker Böhm
 * Version:     1.0
 * ---------------------------------------------------------------------------------------------------
 */

'use strict';

const config = require('./config.json');
const defaultConfig = config.development;
const environment = process.env.NODE_ENV || 'development';
const environmentConfig = config[environment];
const finalConfig = Object.assign(defaultConfig, environmentConfig);

var Server = require('./RS485Server');
var myServer = new Server(finalConfig.RS485Address, finalConfig.serialPort, finalConfig.baudrate);


myServer.createHttpServer(finalConfig.nodePort);

process.on('uncaughtException', function (err){
    console.log(err);
    myServer.close();
    process.exit(1);
 });


