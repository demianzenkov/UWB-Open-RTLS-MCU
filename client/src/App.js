import React from 'react';

import 'jquery';
import 'bootstrap/js/dist/button';
import Select from 'react-select';
import socketIOClient from 'socket.io-client';

import './App.css';
import 'bootstrap/dist/css/bootstrap.css';


function sleep(milliseconds) {
  var start = new Date().getTime();
  for (var i = 0; i < 1e7; i++) {
    if ((new Date().getTime() - start) > milliseconds){
      break;
    }
  }
}
sleep(0);

//const address = '192.168.1.16:3001';
const address = 'localhost:3001';
// const address = '192.168.50.220:3001';
// const address = '192.168.50.220:3001';

const socket = socketIOClient(address, {reconnect: true});

class App extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      endpoint: address, 
      color: 'white',
      consoleRows: 16, 
      availablePorts: [],
      portSelected: [],
      consoleShowData : "",
      consoleInputData : "",
      consoleSendData: "",
    };
    /* Local functions prototypes */
    this.pressedReloadPorts = this.pressedReloadPorts.bind(this);
    this.pressedOpenPort = this.pressedOpenPort.bind(this);
    this.pressedClosePort = this.pressedClosePort.bind(this);
    this.pressedClearConsole = this.pressedClearConsole.bind(this);

    this.portSelectedHandler = this.portSelectedHandler.bind(this);
    this.pressedSend = this.pressedSend.bind(this);

    this.sendTextAdded = this.sendTextAdded.bind(this);

    /* Refs to a DOM node */
    this._refConsoleTextarea = React.createRef();
  }

  pressedReloadPorts(event) {
    //const socket = socketIOClient(this.state.endpoint);
    console.log('socket.emit([app]->(pressedReloadPorts))');
    socket.emit('[app]->(pressedReloadPorts)');
  }

  pressedOpenPort(event) {
    //const socket = socketIOClient(this.state.endpoint);
    socket.emit('[app]->(pressedOpenPort)', {value: this.state.portSelected.value, name: this.state.portSelected.label});
  }

  pressedClosePort(event) {
    //const socket = socketIOClient(this.state.endpoint);
    //console.log('socket.emit([app]->(pressedClosePort))');
    socket.emit('[app]->(pressedClosePort)');
  }

  pressedClearConsole(event) {
    this.setState({consoleShowData: ""});
  }

  sendTextAdded(event) {
    //console.log(event.target.value);
    this.setState({consoleSendData: (event.target.value)});
  }

  pressedSend(event) {
    socket.emit('[app]->(sendData)', { sent: this.state.consoleSendData});
    this.setState({consoleSendData: ""});
  }
  
  componentDidMount = () => {
    
    socket.on('[server]->(availablePorts)', (data) => {
      if (data.ports.length > 0)
      {
        let arr = [];
        data.ports.forEach(function(port, index){
          arr.push({value: index, label: port.comName});
        }, this);
        this.setState({availablePorts: arr});
      }
    });
    
    socket.on('[server]->(portsReloaded)', () => {
      console.log("socket.on([server]->(portsReloaded))");
    });
    
    socket.on('[server]->(portOpened)', () => {
      console.log("socket.on([server]->(portOpened))");
    });

    socket.on('[server]->(portClosed)', () => {
      console.log("socket.on([server]->(portClosed))");
    });

    socket.on('[server]->(availableSerialData)', (data) => {
      // console.log("socket.on([server]->(availableSerialData))");
      // console.log(data);
      this.setState({consoleShowData: this.state.consoleShowData.concat(data)});
      this._refConsoleTextarea.scrollTop = this._refConsoleTextarea.scrollHeight;
    });

    socket.on('diconnect', () =>{
      console.log("Client diconnected by server");
    });

  };


  portSelectedHandler = (portSelected) => {
    this.setState({ portSelected });
    console.log(`Option selected:`, portSelected);
  }
  
  serialTextEnd = React.createRef();

  render() {
    document.body.style.backgroundColor = this.state.color;
    
    return (
      <div className="container-fluid mainContainer">
        <div className="row headerRow">
        </div>
        <div className="row">
          <div className="colSerialConfig col-md-2">
            <div className="divSelectPort">
              <Select placeholder={ "Select port..." }
                      isSearchable={ false }
                      value={ this.state.portSelected }
                      options={ this.state.availablePorts }
                      onChange={ this.portSelectedHandler }
                      
              />
            </div>
            <div className="buttonDiv">
              <button type="button" className="btn btn-block btn-outline-primary" onClick={this.pressedReloadPorts}>Reload ports</button>
              <button type="button" className="btn btn-block btn-outline-success" onClick={this.pressedOpenPort}>Open port</button>
              <button type="button" className="btn btn-block btn-outline-danger" onClick={this.pressedClosePort}>Close port</button>
            </div>
          </div>
          <div className="colConsoleConfig col-md-1"></div>
          <div className="colSerialConsole col-md-7">
            <div className="divConsoleText" >
              <textarea className="form-control textareaConsoleText" 
                        readOnly={true} 
                        value={ this.state.consoleShowData }
                        rows={ this.state.consoleRows }
                        ref={(input) => {this._refConsoleTextarea = input}}
              />
            </div>
            <div className="row">
              <div className="colConsoleSendButton col-md-2">
              <button type="button" className="btn btn-block btn-outline-primary" onClick={this.pressedSend}>Send</button>
              </div>
              <div className="colConsoleInput col-md-10">
                <textarea className="form-control textareaConsoleInput"
                          value={ this.state.consoleSendData }
                          rows={ 1 }
                          onChange = { this.sendTextAdded }
                />
              </div>
            </div>
          </div>
          <div className="colConsoleFeatures col-md-2">
            <div className="buttonDivFeatures">
              <button type="button" 
                      className="btn btn-block btn-outline-secondary " 
                      onClick={this.pressedClearConsole}>Clear
              </button>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

export default App;
