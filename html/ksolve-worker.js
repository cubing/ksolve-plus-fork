var that = this;

var Module = {
  'print': function report(str) {
    that.postMessage(str);
  }
};

importScripts('ksolve.js');

var solve = Module.cwrap('solve', 'void', ['string', 'string']);

this.addEventListener('message', function(e) {
  switch(e.data[0]) {
    case "solve":
      solve(e.data[1], e.data[2]);
      break;
    default:
      Module.print("Bad message.");
      break;
  }
}, false);
