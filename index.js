const dgram = require('dgram');
const server = dgram.createSocket('udp4');

var x = 0;
var y = 0;
var z = -1;
var addr = [];

const COORD = 'x';
const PORT = 41234;

server.on('error', (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

server.on('message', (msg, rinfo) => {
  if(rinfo.address=='127.0.0.1') return;
  switch(String.fromCharCode(msg[0])){
  	case COORD:
  		//console.log("COORD RECEIVED!");
  		//console.log(String.fromCharCode(msg[0]));
		x = .8*x+.2*parseInt_16(msg[1],msg[2]);
		y = .8*y+.2*parseInt_16(msg[3],msg[4]);
		z = .8*z+.2*parseInt_16(msg[5],msg[6]);
		console.log(x + ", " + y + ", " + z);
		break;
  }
  //console.log(`server got: ${msg} from ${rinfo.address}:${rinfo.port}`);
  if(!contains(addr,rinfo.address)){
  	addr.push(rinfo.address);
  	//console.log(addr);
  }
  for(i in addr){
  	var xtemp = parseInt(x);
  	var ytemp = parseInt(y);
  	var ztemp = parseInt(z);
  	msg = new Buffer(['x',xtemp/256,xtemp%256,ytemp/256,ytemp%256,ztemp/256,ztemp%256]);
  	server.send(msg, 0, msg.length, PORT, addr[i]);
  	//console.log("sent '" + msg + "' to: " + addr[i]);
  }
});

server.on('listening', () => {
  const address = server.address();
  //console.log(`server listening ${address.address}:${address.port}`);
});

function parseInt_16(hh,ll){
	val = parseInt(hh)*256+parseInt(ll);
	if(val>0x7FFF) val -= 0x10000;
	return val;
}
function contains(a, obj) {
    var i = a.length;
    while (i--) {
       if (a[i] === obj) {
           return true;
       }
    }
    return false;
}


server.bind(PORT);
// server listening 0.0.0.0:41234
