import React from 'react';

import { BrowserRouter, Route } from "react-router-dom";

import SideNav, { NavItem, NavIcon, NavText } from '@trendmicro/react-sidenav';
import '@trendmicro/react-sidenav/dist/react-sidenav.css';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import { faTerminal } from "@fortawesome/free-solid-svg-icons";

import SerialConsole from './items/serial-console';

class App extends React.Component {
  render() {
    return (
      
        <BrowserRouter>
          <Route render = { ({location, history}) => (
            <React.Fragment>
              <SideNav
                  onSelect={(selected) => {
                      const to = '/' + selected;
                      if (location.pathname !== to) {
                          history.push(to);
                      }
                  }}
              >
                <SideNav.Toggle />
                <SideNav.Nav defaultSelected="serial-сonsole">
                    <NavItem eventKey="serial-console">
                        <NavIcon>
                          <FontAwesomeIcon icon={faTerminal} style={{ fontSize: '1.5em' }}/>
                            {/* <i className="fas fa-home" style={{ fontSize: '1.75em' }} /> */}
                        </NavIcon>
                        <NavText>
                            Serial Console
                        </NavText>
                    </NavItem>
                </SideNav.Nav>
              </SideNav>
              <main>
                {/* <Route path="/" exact component={props => <SerialConsole />} /> */}
                <Route path="/serial-console" component={props => <SerialConsole />} />
              </main>
            </React.Fragment>
          )} />
        </BrowserRouter>
      
      // <SerialConsole />
    );
  }
}

export default App;
