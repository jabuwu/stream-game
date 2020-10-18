const WebSocket = require('ws');

function listen(port) {
  let socketServer = new WebSocket.Server({port, perMessageDeflate: false});
  socketServer.broadcast = function(data) {
    socketServer.clients.forEach(function each(client) {
      if (client.readyState === WebSocket.OPEN) {
        client.send(data);
      }
    });
  };
  return socketServer;
}

module.exports = { listen };