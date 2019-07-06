/* Saga */
import { call, fork, put, takeLatest } from "redux-saga/effects";

function foo() {
  return fetch("http://localhost:8000/anchors").then(response =>
    response.json().then(data => {
      return { body: data, status: response.status };
    })
  );
}

function* requestAnchors(action) {
  const response = yield call(foo);
  console.log(response);
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

export default [fork(watchRequestAnchors)];
