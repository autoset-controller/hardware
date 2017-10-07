let serialport = require('serialport');
let dgram = require('dgram');

let port = new serialport('COM9');
let sendSocket = dgram.createSocket('udp4');

let store = false;
let label = ''; //tracks label of input radio
let breaker = ''; //tracks proper formatting
let value = ''; //tracks value of input
let inputCount = 0;

// Switches the port into "flowing mode"
port.on('data', function (data) {
    let buffSize = data.byteLength;
    let i;
    for (i = 0; i < buffSize; i++) {
        let c = String.fromCharCode(data[i]);
        if(c == '\n') {
            //if input is valid and value is an int
            if(store == true && !isNaN(parseInt(value))) {
                //output new package
                let packageString = 'New Package: \nFrom Radio ' + label + '\n';
                packageString += 'Value = ' + value;
                console.log(packageString);
                if(label == 'A') { //If signal is from radio A
                    sendSocket.send(value, 50000, 'localhost');
                }
                else { //If signal is from radio B
                    sendSocket.send(value, 50001, 'localhost');
                }
            }
            else {
                console.log('invalid serial input: ' + label + breaker + value);
            }
            //reset values
            label = '';
            breaker = '';
            value = '';
            store = false;
        }
        if(label == '') {
            if(c == 'A' || c == 'B') {
                label = c;
                //console.log('Label: ' + label);
            }
            else {

            }
        }
        else if(breaker == '' || breaker == ':') {
            breaker += c;
            //console.log('Breaker: ' + breaker);
        }
        else {
            //if c is not a space, carriage return, or newline
            if(c != '\n' && data[i] != 10 && data[i] != 13) {
                //if value contains 3 or fewer chars and c is a digit
                if(value.length < 4 && !isNaN(parseInt(c))) {
                    value += c;
                    store = true;
                    //console.log('Value: ' + value);
                }
                else {
                    value += c;
                    store = false;
                    //console.log('Invalid Value: ' + value);
                }
            }
        }
    }
});