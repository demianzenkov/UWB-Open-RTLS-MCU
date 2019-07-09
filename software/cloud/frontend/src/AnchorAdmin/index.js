import React, { Component } from "react";
import { connect } from "react-redux";
import { bindActionCreators } from "redux";
import * as actionsModule from "./actions";

class AnchorAdmin extends Component {
  constructor(props) {
    super(props);

    this.props.actions.requestAnchors();
  }

  componentWillReceiveProps(props) {}

  render() {
    const { anchors, isRequestingAnchors } = this.props.state;
    if (isRequestingAnchors) {
      return <div>Loading</div>;
    }

    console.log(">>>>>", anchors, typeof anchors);

    return (
      <div style={{ marginTop: "1rem", marginLeft: "1rem" }}>
        <div style={{ fontSize: "2rem", marginBottom: "0.5rem" }}>
          Anchor Admin
        </div>
        {anchors.map(a => (
          <div key={a.id}>
            <div>
              ip: {a.ip} port: {a.port} server ip: {a.server_ip} server port:{" "}
              {a.server_port} subnet mask: {a.subnet_mask}
            </div>
            <button
              onClick={() => {
                this.props.actions.requestReadNetworkSettings(a.ip, a.port);
              }}
            >
              Read network settings
            </button>
            <button>Write network settings</button>
            <div className="loader" />
          </div>
        ))}
        <input />
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
