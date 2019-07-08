export function requestAnchors() {
  return { type: "anchorAdmin.requestAnchors" };
}

export function requestReadNetworkSettings(ip, port) {
  return { type: "anchorAdmin.requestReadNetworkSettings", ip, port };
}
