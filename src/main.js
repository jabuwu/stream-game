const { createStream } = require('./stream');

(async() => {
  const { PIXI, app, io } = await createStream();

  let bunbun = PIXI.Texture.from('./img/bunny1_stand.png');
  let grass = PIXI.Texture.from('./img/ground_grass_small.png');
  let carrot = PIXI.Texture.from('./img/carrot.png');

  let grounds = [];
  for (let i = 0; i < 40; ++i) {
    let ground = new PIXI.Sprite(grass);
    ground.anchor.set(0.5);
    ground.scale.x = 0.3;
    ground.scale.y = 0.3;
    ground.x = Math.floor(Math.random() * 512);
    ground.y = i * 13;
    app.stage.addChild(ground);
    grounds.push(ground);
  }

  let carrotSprite = new PIXI.Sprite(carrot);
  carrotSprite.scale.x = 0.3;
  carrotSprite.scale.y = 0.3;
  carrotSprite.x = 50;
  carrotSprite.y = 50;
  app.stage.addChild(carrotSprite);

  io.on('connection', function (socket) {
    let text = new PIXI.Text('BunBun', { fontFamily: 'Arial', fontSize: 18, fill: 0x000000, align: 'center' });
    text.anchor.set(0.5);
    app.stage.addChild(text);

    let sprite = new PIXI.Sprite(bunbun);
    sprite.anchor.set(0.5);

    app.stage.addChild(sprite);
    sprite.scale.x = 0.3;
    sprite.scale.y = 0.3;
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
      let ground = 483;
      for (let g of grounds) {
        let gg = g.y - 45;
        if (sprite.y <= gg && sprite.x > g.x - 40 && sprite.x < g.x + 40 && ground > gg) {
          ground = gg;
        }
      }
      yspeed += 1;
      sprite.y += yspeed;
      if (sprite.y > ground) {
        yspeed = 0;
        sprite.y = ground;
      }
      if (keysDown[32] && sprite.y == ground) {
        yspeed = -15;
      }
      if (keysDown[68]) {
        sprite.x += 5;
      }
      if (keysDown[65]) {
        sprite.x -= 5;
      }
      text.x = sprite.x;
      text.y = sprite.y - 40;
      let dist = Math.sqrt((sprite.x - carrotSprite.x) * (sprite.x - carrotSprite.x) + (sprite.y - carrotSprite.y) * (sprite.y - carrotSprite.y));
      if (dist < 30) {
        carrotSprite.x = Math.floor(Math.random() * 512);
        carrotSprite.y = Math.floor(Math.random() * 512);
      }
    }, 16);
  });
})();