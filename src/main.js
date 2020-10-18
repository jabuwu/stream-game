const { createStream } = require('./stream');

(async() => {
  const { PIXI, app, io } = await createStream();

  let bunbun = PIXI.Texture.from('./img/bunny1_stand.png');

  io.on('connection', function (socket) {
    let text = new PIXI.Text('BunBun', { fontFamily: 'Arial', fontSize: 18, fill: 0x000000, align: 'center' });
    text.anchor.set(0.5);
    app.stage.addChild(text);

    let sprite = new PIXI.Sprite(bunbun);
    sprite.anchor.set(0.5);

    app.stage.addChild(sprite);
    sprite.scale.x = 0.2;
    sprite.scale.y = 0.2;
    sprite.x = 80;
    sprite.y = 80;

    let yspeed = 0;

    let keysDown = {};
    socket.on('onkeydown', (key) => {
      keysDown[key] = true;
    });
    socket.on('onkeyup', (key) => {
      keysDown[key] = false;
    });
    socket.on('disconnect', () => {
      app.stage.removeChild(sprite);
      app.stage.removeChild(text);
    });
    setInterval(() => {
      let ground = 492;
      yspeed += 1;
      sprite.y += yspeed;
      if (sprite.y > ground) {
        yspeed = 0;
        sprite.y = ground;
      }
      if (keysDown[32] && sprite.y == ground) {
        yspeed = -10;
      }
      if (keysDown[68]) {
        sprite.x += 5;
      }
      if (keysDown[65]) {
        sprite.x -= 5;
      }
      text.x = sprite.x;
      text.y = sprite.y - 40;
    }, 16);
  });
})();