import React, { Component } from "react";
import { Provider } from "react-redux";
import { createStore, applyMiddleware } from "redux";
import { createLogger } from "redux-logger";
import { take, put, call, fork, delay } from "redux-saga/effects";
import createSagaMiddleware, { eventChannel } from "redux-saga";
import Monitor from "./Monitor";

const logger = createLogger({});

const reducer = (state = {}, action) => {
  switch (action.type) {
    case "ws.message":
      return {
        dots: [
          ...state.dots.filter(d => d.k !== action.message.k),
          action.message
        ]
      };
    default:
      return state;
  }
};

function setDelay() {
  return Math.round(5 + Math.random() * 10) * 1000;
}

function createWebSocketChannel() {
  return eventChannel(emit => {
    const socket = new WebSocket("ws://127.0.0.1:8080/ws");

    socket.onmessage = wsMessage => {
      const message = JSON.parse(wsMessage.data);
      switch (message.type) {
        default:
          emit({ message });
      }
    };

    socket.onclose = () => {
      emit({ close: true });
    };

    return () => {};
  });
}

function* listenToWebSocket(delayTimeout) {
  yield delay(delayTimeout);
  const webSocketChannel = yield call(createWebSocketChannel);

  while (true) {
    const { message, close } = yield take(webSocketChannel);

    if (message) {
      yield put({ type: "ws.message", message });
    }
    if (close) {
      webSocketChannel.close();
    }
  }
}

function* watchOnWebSocket() {
  yield call(listenToWebSocket, 0);
  while (true) {
    yield call(listenToWebSocket, setDelay());
  }
}

const sagaMiddleware = createSagaMiddleware();

const store = createStore(
  reducer,
  { dots: [] },
  applyMiddleware(logger, sagaMiddleware)
);

sagaMiddleware.run(watchOnWebSocket);

class App extends Component {
  render() {
    return (
      <Provider store={store}>
        <div className="App">
          <Monitor />
        </div>
      </Provider>
    );
  }
}

export default App;
