function bar() {
  print("b\n");
}

module.exports = {
  foo: function () {bar();}
}
