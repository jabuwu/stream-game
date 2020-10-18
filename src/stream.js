const path = require('path');
const http = require('http');
const express = require('express');
const SocketIO = require('socket.io');
const { listen } = require('./websocket');
const { createPixi } = require('./pixi');
const { addFrame } = require('../build/Release/addon');

async function createStream(port, wsPort) {
  port = port || 8000;
  wsPort = wsPort || 8082;

  let PIXI = await createPixi();
  let dom = PIXI.dom;
  let app = new PIXI.Application({
    width: 512,
    height: 512,
    forceCanvas: true,
    preserveDrawingBuffer: true
  });
  app.renderer.backgroundColor = 0xa7fbfc;
  dom.window.document.body.appendChild(app.view);
  dom.window.app = app;
  await new Promise(resolve => setTimeout(resolve, 100));
  let canvas = dom.window.document.getElementsByTagName('canvas')[0];
  let ctx = canvas.getContext('2d');

  let expressApp = express();
  expressApp.use(express.static(path.join(__dirname, '../static')));
  let server = http.createServer(expressApp);
  let io = SocketIO.listen(server);
  server.listen(port);

  let ws = listen(wsPort);

  setInterval(() => {
    ws.broadcast(addFrame(ctx.getImageData(0, 0, 512, 512).data));
  }, 20);

  return { PIXI, app, io };
}

module.exports = { createStream };