import React, { Component } from "react";
import { connect } from "react-redux";

class Monitor extends Component {
  constructor(props) {
    super(props);

    this.state = {
      dots: []
    };
  }

  componentWillReceiveProps(props) {
    this.setState(props);
  }

  render() {
    return (
      <div>
        <svg
          width="1000"
          height="500"
          style={{
            backgroundColor: "yellow",
            marginTop: "6rem",
            marginLeft: "2rem",
            border: "1px solid black"
          }}
        >
          {this.state.dots.map(d => (
            <circle key={d.k} cx={d.x} cy={d.y} r="5px" />
          ))}
        </svg>
      </div>
    );
  }
}

export default connect(state => state)(Monitor);
