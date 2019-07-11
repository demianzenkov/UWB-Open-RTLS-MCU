const defaultState = {
  anchors: [],
  isRequestingAnchors: false
};

const reducer = (state = defaultState, action) => {
  switch (action.type) {
    case "anchorAdmin.requestAnchors":
      return { ...state, isRequestingAnchors: true };
    case "anchorAdmin.ws.anchors":
    case "anchorAdmin.requestAnchors.success":
      return { ...state, isRequestingAnchors: false, anchors: action.data };
    case "anchorAdmin.requestReadNetworkSettings.success":
    case "anchorAdmin.requestDeleteAnchor.success":
      return { ...state, anchors: action.data };
    default:
      return state;
  }
};

export default reducer;
