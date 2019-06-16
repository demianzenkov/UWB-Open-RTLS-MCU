import React, { Component } from 'react';

class Monitor extends Component {
  constructor(props) {
    super(props)

    this.state = {'x': 0}

    setInterval(()=>{
      this.setState({'x': this.state.x + 1})
      if (this.state.x > 100) {
        this.setState({'x': 0})
      }
    }, 10)
  }

  componentDidMount() {
  }

  render() {
    return (
      <div>
        <svg width="500" height="500" style={{'backgroundColor': 'yellow'}}>
          <circle cx={this.state.x} cy="25" r='5'></circle>
        </svg>
      </div>
    );
  }
}

export default Monitor;