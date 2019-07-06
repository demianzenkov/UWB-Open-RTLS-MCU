const defaultState = {
  anchors: [],
  isRequestingAnchors: false
};

const reducer = (state = defaultState, action) => {
  switch (action.type) {
    case "anchorAdmin.requestAnchors":
      return { ...state, isRequestingAnchors: true };
    case "anchorAdmin.requestAnchors.success":
      return { ...state, isRequestingAnchors: false, anchors: action.data };
    default:
      return state;
  }
};

export default reducer;
