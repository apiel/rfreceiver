var spawn = require('child_process').spawn;

function codeParse(code) {
    prevBit = 0;
    sender = 0;
    recipient = 0;
    bitCount = 0;
    command = 0;
    group = 0;

    for(var bit of code) {
        if(bitCount % 2 == 1)
        {
            if((prevBit ^ bit) == 0)
            {	// must be either 01 or 10, cannot be 00 or 11
                // cout << "must be either 01 or 10, cannot be 00 or 11, normally get out" << endl;
                break;
            }
            else if(bitCount < 53)
            {	// first 26 data bits
                
                sender <<= 1;
                sender |= prevBit;
            }
            else if(bitCount == 53)
            {	// 26th data bit
                //console.log('Sender: ' + sender);
                group = prevBit;
            }
            else if(bitCount == 55)
            {	// 27th data bit
                //console.log('Group: ' + group);
                command = prevBit;
            }
            else
            {	// last 4 data bits
                recipient <<= 1;
                recipient |= prevBit;
            }
        }
        
        prevBit = bit;
        bitCount++;
        
        if(bitCount == 64)
        {	// message is complete
            console.log("sender: " + sender +
                        " recipient: " + recipient +
                        " command: " + command +
                        " group: " + group);
            break;
        }
    }
}

const rfreceiver = spawn('./rfreceiver');

rfreceiver.stdout.on('data', function (data) {
    let value = data.toString().trim();
    let code = value.split(" ")[1];
    //console.log(code);
    codeParse(code);
});

rfreceiver.stderr.on('data', function (data) {
    console.log('stderr: ' + data.toString());
});

rfreceiver.on('exit', function (code) {
    console.log('child process exited with code ' + code.toString());
});
