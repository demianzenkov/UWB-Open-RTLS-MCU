import React, { Component } from "react";
import { Provider } from "react-redux";
import Monitor from "./Monitor";
import AnchorAdmin from "./AnchorAdmin";
import store from "./setupStore";

class App extends Component {
  render() {
    return (
      <Provider store={store}>
        <div className="App">
          <Monitor />
          <AnchorAdmin />
        </div>
      </Provider>
    );
  }
}

export default App;
