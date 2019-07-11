export function requestAnchors() {
  return { type: "anchorAdmin.requestAnchors" };
}

export function requestReadNetworkSettings(ip, port) {
  return { type: "anchorAdmin.requestReadNetworkSettings", ip, port };
}

export function requestDeleteAnchor(ip, port) {
  return { type: "anchorAdmin.requestDeleteAnchor", ip, port };
}
