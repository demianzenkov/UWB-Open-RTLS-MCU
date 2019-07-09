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

  componentWillReceiveProps(props) {}

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
                  onChange={e => {
                    this.setState(
                      {
                        forms: {
                          ...this.state.forms,
                          [a.id]: { ip: e.target.value }
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
            <div className="anchor">
              <div>port:</div>
              <div style={{ display: "flex" }}>
                <div>{a.port}</div>
                <input className="anchor-admin-input" onChange={e => {}} />
              </div>
            </div>
            <div className="anchor">
              <div>server ip:</div>
              <div style={{ display: "flex" }}>
                <div>{a.server_ip}</div>
                <input className="anchor-admin-input" />
              </div>
            </div>
            <div className="anchor">
              <div>server port:</div>
              <div style={{ display: "flex" }}>
                <div>{a.server_port}</div>
                <input className="anchor-admin-input" />
              </div>
            </div>
            <div className="anchor">
              <div>subnet mask:</div>
              <div style={{ display: "flex" }}>
                <div>{a.subnet_mask}</div>
                <input className="anchor-admin-input" />
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
              <button>Clear</button>
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
