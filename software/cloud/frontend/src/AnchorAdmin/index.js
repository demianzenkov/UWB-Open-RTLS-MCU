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
    console.log(this.props);
    return (
      <div>
        <div>Anchor Admin</div>
        {anchors.map(a => (
          <div key={a.id}>
            {a.ip} {a.port} {a.server_ip} {a.server_port} {a.subnet_mask}
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
