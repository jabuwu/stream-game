const JSDOM = require( 'jsdom' );
const jsdom = JSDOM.JSDOM;
const { implSymbol } = require('jsdom/lib/jsdom/living/generated/utils.js');
const { writeFileSync, unlinkSync } = require('fs');

function polyfill(dom) {
  const map = {};
  dom.window.scrollTo = () => {};
  dom.window.URL.createObjectURL = (blob) => {
      const uuid = Math.random().toString(36).slice(2);
      const path = `node_modules/.cache/${uuid}.png`;
      writeFileSync(path, blob[implSymbol]._buffer);
      const url = `file://${path}`;
      map[url] = path;
      return url;
  };
  dom.window.URL.revokeObjectURL = (url) => {
      unlinkSync(map[url]);
      delete map[url];
  };
}

async function createPixi() {
  let dom = await jsdom.fromFile('./assets/index.html', {
    runScripts: 'dangerously',
    resources: 'usable',
    pretendToBeVisual: true
  });
  polyfill(dom);
  await new Promise(resolve => setTimeout(resolve, 100));
  dom.window.PIXI.dom = dom;
  return dom.window.PIXI;
}

module.exports = { createPixi };