import React, { Component } from "react";
import { connect } from "react-redux";
import { bindActionCreators } from "redux";
import * as actionsModule from "./actions";

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
        {anchors.map(a => (
          <div key={a.id}>
            <div className="anchor">
              <div>ip:</div>
              <div style={{ display: "flex" }}>
                <div>{a.ip}</div>
                <input
                  className="anchor-admin-input"
                  type="text"
                  value={this.state[a.id]["ip"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: { ...this.state[1], ip: e.target.value }
                    });
                  }}
                />
              </div>
            </div>
            <div className="anchor">
              <div>port:</div>
              <div style={{ display: "flex" }}>
                <div>{a.port}</div>
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["port"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: { ...this.state[1], port: e.target.value }
                    });
                  }}
                />
              </div>
            </div>
            <div className="anchor">
              <div>server ip:</div>
              <div style={{ display: "flex" }}>
                <div>{a.server_ip}</div>
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["server ip"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: { ...this.state[1], "server ip": e.target.value }
                    });
                  }}
                />
              </div>
            </div>
            <div className="anchor">
              <div>server port:</div>
              <div style={{ display: "flex" }}>
                <div>{a.server_port}</div>
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["server port"]}
                  onChange={e => {
                    this.setState({
                      [a.id]: {
                        ...this.state[1],
                        "server port": e.target.value
                      }
                    });
                  }}
                />
              </div>
            </div>
            <div className="anchor">
              <div>subnet mask:</div>
              <div style={{ display: "flex" }}>
                <div>{a.subnet_mask}</div>
                <input
                  className="anchor-admin-input"
                  value={this.state[a.id]["subnet mask"]}
                  onChange={e => {
                    this.setState(
                      {
                        [a.id]: {
                          ...this.state[1],
                          "subnet mask": e.target.value
                        }
                      },
                      () => {
                        console.log(this.state);
                      }
                    );
                  }}
                />
              </div>
            </div>
            <div
              style={{
                display: "flex",
                justifyContent: "space-between",
                width: "25rem"
              }}
            >
              <button
                onClick={() => {
                  this.props.actions.requestReadNetworkSettings(a.ip, a.port);
                }}
              >
                Read network settings
              </button>
              <button>Write network settings</button>
              <button
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
