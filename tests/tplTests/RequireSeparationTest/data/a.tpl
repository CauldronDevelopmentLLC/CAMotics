function bar() {
  print("a\n");
}

module.exports = {
  foo: function () {bar();}
}
