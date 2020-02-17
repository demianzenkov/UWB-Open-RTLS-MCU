import { createStore, applyMiddleware, combineReducers } from "redux";
import { createLogger } from "redux-logger";
import { take, put, call, delay, all, fork } from "redux-saga/effects";
import createSagaMiddleware, { eventChannel } from "redux-saga";

import { default as monitorReducer } from "./Monitor/reducer";
import { default as anchorAdminReducer } from "./AnchorAdmin/reducer";

import { default as anchorAdminSaga } from "./AnchorAdmin/sagas";

const logger = createLogger({});

const rootReducer = combineReducers({
  monitor: monitorReducer,
  anchorAdmin: anchorAdminReducer
});

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
      const { type } = message;
      if (type === "anchors") {
        // const data = JSON.parse(message.data);
        yield put({ type: "anchorAdmin.ws.anchors", data: message.data });
      } else {
        yield put({ type: "ws.message", message });
      }
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

export function* rootSaga() {
  yield all([...anchorAdminSaga, fork(watchOnWebSocket)]);
}

const sagaMiddleware = createSagaMiddleware();

const store = createStore(
  rootReducer,
  {},
  applyMiddleware(logger, sagaMiddleware)
);

sagaMiddleware.run(rootSaga);

export default store;
