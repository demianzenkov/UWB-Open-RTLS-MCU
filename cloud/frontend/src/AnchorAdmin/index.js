import React, { Component } from "react";
import { connect } from "react-redux";
import { bindActionCreators } from "redux";
import * as actionsModule from "./actions";

function validateIPaddress(ipaddress) {
  if (
    /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(
      ipaddress
    )
  ) {
    return true;
  }
  return false;
}

function validatePayload(payload) {
  var isValid = true;

  if (
    !validateIPaddress(payload["ip"]) ||
    !validateIPaddress(payload["server ip"]) ||
    !validateIPaddress(payload["subnet mask"])
  ) {
    isValid = false;
  }

  if (isNaN(Number(payload["port"])) || isNaN(Number(payload["server port"]))) {
    isValid = false;
  }

  return isValid;
}

class AnchorAdmin extends Component {
  constructor(props) {
    super(props);

    this.state = {};

    this.props.actions.requestAnchors();
  }

  componentWillReceiveProps(props) {
    if (!!props) {
      props.state.anchors.forEach(a => {
        if (!Object.keys(props.state.anchors).includes(a.id)) {
          this.setState({
            [a.id]: {
              ip: "",
              port: "",
              "server ip": "",
              "server port": "",
              "subnet mask": ""
            }
          });
        }
      });
    }
  }

  render() {
    const { anchors, isRequestingAnchors } = this.props.state;
    if (isRequestingAnchors) {
      return <div>Loading</div>;
    }

    return (
      <div style={{ marginTop: "1rem", marginLeft: "1rem" }}>
        <div style={{ fontSize: "2rem", marginBottom: "0.5rem" }}>
          Anchor Admin
        </div>
        {anchors.length === 0 && <div>No anchors</div>}
        {anchors.map(a => (
          <div key={a.id}>
            <div style={{ display: "flex" }}>
              <div key={a.id}>
                <div className="anchor">
                  <div>ip:</div>
                  <div>{a.ip}</div>
                </div>
                <div className="anchor">
                  <div>port:</div>
                  <div>{a.port}</div>
                </div>
                <div className="anchor">
                  <div>server ip:</div>
                  <div>{a.server_ip}</div>
                </div>
                <div className="anchor">
                  <div>server port:</div>
                  <div>{a.server_port}</div>
                </div>
                <div className="anchor">
                  <div>subnet mask:</div>
                  <div>{a.subnet_mask}</div>
                </div>
              </div>
              <div style={{ display: "flex", flexDirection: "column" }}>
                <input
                  className="anchor-admin-input"
                  type="text"
                  value={this.state[a.id]["ip"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: { ...this.state[a.id], ip: e.target.value }
                    });
                  }}
                />
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["port"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: { ...this.state[a.id], port: e.target.value }
                    });
                  }}
                />
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["server ip"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: {
                        ...this.state[a.id],
                        "server ip": e.target.value
                      }
                    });
                  }}
                />
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["server port"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: {
                        ...this.state[a.id],
                        "server port": e.target.value
                      }
                    });
                  }}
                />
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["subnet mask"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: {
                        ...this.state[a.id],
                        "subnet mask": e.target.value
                      }
                    });
                  }}
                />
              </div>
              <div style={{ display: "flex", flexDirection: "column" }}>
                <button
                  style={{ marginLeft: "0.25rem" }}
                  disabled={
                    a["is_waiting_for_write_network_settings_command_response"]
                  }
                  onClick={() => {
                    if (!validatePayload(this.state[a.id])) {
                      this.setState({
                        [a.id]: {
                          ...this.state[a.id],
                          shouldDisplayWriteError: true
                        }
                      });
                    } else {
                      this.props.actions.requestWriteNetworkSettings(
                        a.ip,
                        a.port,
                        this.state[a.id]
                      );
                    }
                    setTimeout(() => {
                      this.setState({
                        [a.id]: {
                          ...this.state[a.id],
                          shouldDisplayWriteError: false
                        }
                      });
                    }, 5000);
                  }}
                >
                  Write network settings
                </button>
                {this.state[a.id]["shouldDisplayWriteError"] && (
                  <div style={{ marginLeft: "0.25rem", fontSize: "0.8rem" }}>
                    Wrong input!
                  </div>
                )}
                {a[
                  "is_waiting_for_write_network_settings_command_response"
                ] && (
                  <div style={{ marginLeft: "0.25rem", fontSize: "0.8rem" }}>
                    Waititng for response
                  </div>
                )}
                <button
                  style={{ marginLeft: "0.25rem", marginTop: "0.25rem" }}
                  onClick={() => {
                    this.setState({
                      [a.id]: {
                        ip: "",
                        port: "",
                        "server ip": "",
                        "server port": "",
                        "subnet mask": ""
                      }
                    });
                  }}
                >
                  Clear
                </button>
                <button
                  style={{ marginLeft: "0.25rem", marginTop: "0.25rem" }}
                  onClick={() => {
                    this.props.actions.requestDeleteAnchor(a.ip, a.port);
                  }}
                >
                  Delete
                </button>
              </div>
            </div>
            <div
              style={{
                display: "flex",
                justifyContent: "space-between",
                width: "25rem"
              }}
            >
              <div style={{ display: "flex" }}>
                <button
                  style={{ marginTop: "0.25rem" }}
                  onClick={() => {
                    this.props.actions.requestReadNetworkSettings(a.ip, a.port);
                  }}
                  disabled={
                    a["is_waiting_for_read_network_settings_command_response"]
                  }
                >
                  Read network settings
                </button>
                {a["is_waiting_for_read_network_settings_command_response"] && (
                  <div
                    style={{
                      fontSize: "0.8rem",
                      marginTop: "0.4rem",
                      marginLeft: "0.5rem"
                    }}
                  >
                    Waiting for response
                  </div>
                )}
              </div>
            </div>
          </div>
        ))}
      </div>
    );
  }
}

export default connect(
  state => ({
    state: state.anchorAdmin
  }),
  dispatch => ({ actions: bindActionCreators(actionsModule, dispatch) })
)(AnchorAdmin);
