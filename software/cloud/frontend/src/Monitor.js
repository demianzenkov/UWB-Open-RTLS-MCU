import React, { Component } from "react";

class Monitor extends Component {
  constructor(props) {
    super(props);

    this.state = {
      dots: [
        { k: 1, x: 250, y: 90, vX: -1, vY: -1 },
        { k: 2, x: 150, y: 45, vX: -1, vY: 1 },
        { k: 3, x: 900, y: 70, vX: 1, vY: -1 },
        { k: 4, x: 700, y: 10, vX: 1, vY: 1 },
        { k: 5, x: 900, y: 120, vX: 1, vY: -1 },
        { k: 6, x: 200, y: 50, vX: 1, vY: 1 }
      ]
    };

    setInterval(() => {
      this.state.dots.forEach(d => {
        if (d.x + d.vX === 1000) {
          const newDot = { k: d.k, x: 999, y: d.y, vY: d.vY, vX: -1 };
          this.setState({
            dots: [...this.state.dots.filter(dd => dd.k !== d.k), newDot]
          });
          return;
        }

        if (d.x + d.vX === 0) {
          const newDot = { k: d.k, x: 1, y: d.y, vY: d.vY, vX: 1 };
          this.setState({
            dots: [...this.state.dots.filter(dd => dd.k !== d.k), newDot]
          });
          return;
        }

        if (d.y + d.vY === 500) {
          const newDot = { k: d.k, x: d.x, y: d.y, vY: -1, vX: d.vX };
          this.setState({
            dots: [...this.state.dots.filter(dd => dd.k !== d.k), newDot]
          });
          return;
        }

        if (d.y + d.vY === 0) {
          const newDot = { k: d.k, x: d.x, y: d.y, vY: 1, vX: d.vX };
          this.setState({
            dots: [...this.state.dots.filter(dd => dd.k !== d.k), newDot]
          });
          return;
        }

        const newDot = {
          k: d.k,
          x: d.x + d.vX,
          y: d.y + d.vY,
          vY: d.vY,
          vX: d.vX
        };
        this.setState({
          dots: [...this.state.dots.filter(dd => dd.k !== d.k), newDot]
        });
      });
    }, 10);
  }

  componentDidMount() {}

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

export default Monitor;
