var osc = require("node-osc");

const express = require("express");
const http = require("http");
const socketio = require("socket.io");
const ip = require("ip");

const Readline = require("@serialport/parser-readline");
//const Ready = require('@serialport/parser-ready')
const ByteLength = require("@serialport/parser-byte-length");

const port = 3001;
//const address = '192.168.50.220';
//const address = '192.168.1.7';

const address = "0.0.0.0"; // rpi
// const address = 'localhost';

const app = express();
var SerialPort = require("serialport");

const server = http.createServer(app);

const io = socketio(server); // create socket using instance of http server

var client = new osc.Client("localhost", 3333);
client.send("/oscAddress", 200, function() {
  client.kill();
});

let portOpenedFlag = false;
let serial_port;

io.on("connection", socket => {
  console.log("New client connected");
  //console.log("IP adress: ", ip.address());

  reloadPorts(socket);

  socket.on("[app]->(pressedReloadPorts)", () => {
    reloadPorts(socket);
    console.log("socket.on([app]->(pressedReloadPorts))");
    socket.emit("[server]->(portsReloaded)");
    console.log("socket.emit([server]->(portsReloaded))");
  });

  socket.on("[app]->(pressedOpenPort)", port_data => {
    console.log("[app]->(pressedOpenPort)");
    if (port_data.name != undefined) {
      console.log("Open port: " + port_data.name);
      if (portOpenedFlag) {
        console.log("Port already opened");
        return;
      } else {
        serial_port = new SerialPort(port_data.name, {
          baudRate: 115200,
          autoOpen: false
        });
        serial_port.open(function(err) {
          if (err) {
            return console.log("Error opening port: ", err.message);
          }
        });
        serial_port.on("open", function() {
          portOpenedFlag = true;
          console.log("Port opened!");
          socket.emit("[server]->(portOpened)");
          //const parser = serial_port.pipe(new Readline({ delimiter: '\r\n' }))
          const parser = serial_port.pipe(new Readline());
          //const parser = serial_port.pipe(new ByteLength({length: 1}))

          //serial_port.write(Buffer.from([0x0D, 0x0D]));

          parser.on("data", function(data) {
            //console.log(data);
            io.emit("[server]->(availableSerialData)", data);
            //console.log(data.toString('hex'));
            //io.emit('[server]->(availableSerialData)', data.toString());
          });
        });
        serial_port.on("close", function() {
          portOpenedFlag = false;
          console.log("Port closed!");
        });
      }
    } else {
      console.log("Port name undefined");
    }
  });

  socket.on("[app]->(sendData)", data => {
    if (serial_port != undefined) {
      serial_port.write(Buffer.from(data.sent));
      console.log("[app]->(sendData): " + data.sent);
    }
  });

  socket.on("[app]->(pressedClosePort)", port_data => {
    if (serial_port != undefined) {
      if (serial_port.isOpen) {
        serial_port.close();
        socket.emit("[server]->(portClosed)");
      } else {
        console.log("Port not opened");
      }
    }
  });

  socket.on("diconnect", () => {
    console.log("Client diconnected");
  });
});

function reloadPorts(socket) {
  SerialPort.list(function(err, ports) {
    if (Boolean(ports) && ports.length > 0) {
      socket.emit("[server]->(availablePorts)", { ports: ports });
    } else {
      var out_name = "No port avaialble";
      console.log("[server]->[comName]: " + out_name);
    }
  });
}

server.listen(port, address, () => console.log("Listening on port:", port));
