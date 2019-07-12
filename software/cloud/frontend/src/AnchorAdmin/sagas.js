/* Saga */
import { call, fork, put, takeLatest } from "redux-saga/effects";

function fetchAnchors() {
  return fetch("http://localhost:8000/anchors").then(response =>
    response.json().then(data => {
      return { body: data, status: response.status };
    })
  );
}

function* requestAnchors(action) {
  const response = yield call(fetchAnchors);
  if (response.status === 200) {
    yield put({
      type: "anchorAdmin.requestAnchors.success",
      data: response.body
    });
  } else {
    yield put({
      type: "anchorAdmin.requestAnchors.fail",
      status: response.status
    });
  }
}

function* watchRequestAnchors() {
  yield takeLatest("anchorAdmin.requestAnchors", requestAnchors);
}

function fetchRequestReadNetworkSettings(ip, port) {
  return fetch("http://localhost:8000/anchors/read_network_settings_command", {
    method: "POST",
    body: JSON.stringify({ ip, port }),
    headers: {
      "Content-Type": "application/json"
    }
  }).then(response =>
    response.json().then(data => {
      return { body: data, status: response.status };
    })
  );
}

function* requestReadNetworkSettings(action) {
  const response = yield call(
    fetchRequestReadNetworkSettings,
    action.ip,
    action.port
  );

  if (response.status === 200) {
    yield put({
      type: "anchorAdmin.requestReadNetworkSettings.success",
      data: response.body
    });
  } else {
    yield put({
      type: "anchorAdmin.requestReadNetworkSettings.fail",
      status: response.status
    });
  }
}

function* watchRequestNetworkSettings() {
  yield takeLatest(
    "anchorAdmin.requestReadNetworkSettings",
    requestReadNetworkSettings
  );
}

function fetchRequestDeleteAnchor(ip, port) {
  return fetch("http://localhost:8000/anchors/anchor_deletion", {
    method: "POST",
    body: JSON.stringify({ ip, port }),
    headers: {
      "Content-Type": "application/json"
    }
  }).then(response =>
    response.json().then(data => {
      return { body: data, status: response.status };
    })
  );
}

function* requestDeleteAnchor(action) {
  const response = yield call(fetchRequestDeleteAnchor, action.ip, action.port);

  if (response.status === 200) {
    yield put({
      type: "anchorAdmin.requestDeleteAnchor.success",
      data: response.body
    });
  } else {
    yield put({
      type: "anchorAdmin.requestDeleteAnchor.fail",
      status: response.status
    });
  }
}

function* watchRequestDeleteAnchor() {
  yield takeLatest("anchorAdmin.requestDeleteAnchor", requestDeleteAnchor);
}

function fetchRequestWriteNetworkSettings(ip, port) {
  return fetch("http://localhost:8000/anchors/write_network_settings_command", {
    method: "POST",
    body: JSON.stringify({ ip, port }),
    headers: {
      "Content-Type": "application/json"
    }
  }).then(response =>
    response.json().then(data => {
      return { body: data, status: response.status };
    })
  );
}

function* requestWriteNetworkSettings(action) {
  const response = yield call(
    fetchRequestWriteNetworkSettings,
    action.ip,
    action.port
  );

  if (response.status === 200) {
    yield put({
      type: "anchorAdmin.requestWriteNetworkSettings.success",
      data: response.body
    });
  } else {
    yield put({
      type: "anchorAdmin.requestWriteNetworkSettings.fail",
      status: response.status
    });
  }
}

function* watchRequestWriteNetworkSettings() {
  yield takeLatest(
    "anchorAdmin.requestWriteNetworkSettings",
    requestWriteNetworkSettings
  );
}

export default [
  fork(watchRequestAnchors),
  fork(watchRequestNetworkSettings),
  fork(watchRequestDeleteAnchor),
  fork(watchRequestWriteNetworkSettings)
];
