description("Test behaviour of strict mode");

var globalThisTest;
function testThis() {
    "use strict";
    return this;
}
function testThisDotAccess() {
    "use strict";
    return this.length;
}
function testThisBracketAccess(prop) {
    "use strict";
    return this[prop];
}
function testGlobalAccess() {
    return testThis();
}
shouldBe("testThis.call(null)", "null");
shouldBe("testThis.call(1)", "1");
shouldBe("testThis.call(true)", "true");
shouldBe("testThis.call(false)", "false");
shouldBe("testThis.call(undefined)", "undefined");
shouldBe("testThis.call('a string')", "'a string'");
shouldBe("testThisDotAccess.call('a string')", "'a string'.length");
shouldThrow("testThisDotAccess.call(null)");
shouldThrow("testThisDotAccess.call(undefined)");
shouldBeUndefined("testThisDotAccess.call(true)");
shouldBeUndefined("testThisDotAccess.call(false)");
shouldBeUndefined("testThisDotAccess.call(1)");
shouldBe("testThisBracketAccess.call('a string', 'length')", "'a string'.length");
shouldThrow("testThisBracketAccess.call(null, 'length')");
shouldThrow("testThisBracketAccess.call(undefined, 'length')");
shouldBeUndefined("testThisBracketAccess.call(true, 'length')");
shouldBeUndefined("testThisBracketAccess.call(false, 'length')");
shouldBeUndefined("testThisBracketAccess.call(1, 'length')");
shouldBeUndefined("Function('\"use strict\"; return this;')()");
shouldThrow("Function('\"use strict\"; with({});')");


shouldBe("testGlobalAccess()", "undefined");
shouldBe("testThis.call()", "undefined");
shouldBe("testThis.apply()", "undefined");
shouldBe("testThis.call(undefined)", "undefined");
shouldBe("testThis.apply(undefined)", "undefined");

shouldThrow("(function eval(){'use strict';})");
shouldThrow("(function (eval){'use strict';})");
shouldThrow("(function arguments(){'use strict';})");
shouldThrow("(function (arguments){'use strict';})");
shouldThrow("(function (){'use strict'; var eval;})");
shouldThrow("(function (){'use strict'; var arguments;})");
shouldThrow("(function (){'use strict'; try{}catch(eval){}})");
shouldThrow("(function (){'use strict'; try{}catch(arguments){}})");
shouldThrow("(function (a, a){'use strict';})");
shouldThrow("(function (a){'use strict'; delete a;})()");
shouldThrow("(function (){'use strict'; var a; delete a;})()");
shouldThrow("(function (){var a; function f() {'use strict'; delete a;} })()");
shouldThrow("(function (){'use strict'; with(1){};})");
shouldThrow("(function (){'use strict'; arguments.callee; })()");
shouldThrow("(function (){'use strict'; arguments.caller; })()");
shouldThrow("(function f(){'use strict'; f.arguments; })()");
shouldThrow("(function f(){'use strict'; f.caller; })()");
shouldThrow("(function f(){'use strict'; f.arguments=5; })()");
shouldThrow("(function f(){'use strict'; f.caller=5; })()");
shouldThrow("(function (arg){'use strict'; arguments.callee; })()");
shouldThrow("(function (arg){'use strict'; arguments.caller; })()");
shouldThrow("(function f(arg){'use strict'; f.arguments; })()");
shouldThrow("(function f(arg){'use strict'; f.caller; })()");
shouldThrow("(function f(arg){'use strict'; f.arguments=5; })()");
shouldThrow("(function f(arg){'use strict'; f.caller=5; })()");
shouldThrow("'use strict'; (function (){with(1){};})");
shouldThrow("'use strict'; (function (){var a; delete a;})");
shouldThrow("'use strict'; var a; (function (){ delete a;})");
shouldThrow("var a; (function (){ 'use strict'; delete a;})");
shouldThrow("'misc directive'; 'use strict'; with({}){}");
shouldThrow("'use strict'; return");
shouldThrow("'use strict'; break");
shouldThrow("'use strict'; continue");
shouldThrow("'use strict'; for(;;)return");
shouldThrow("'use strict'; for(;;)break missingLabel");
shouldThrow("'use strict'; for(;;)continue missingLabel");
shouldThrow("'use strict'; 007;");
shouldThrow("'use strict'; '\\007';");
shouldThrow("'\\007'; 'use strict';");

var someDeclaredGlobal;
aDeletableProperty = 'test';

shouldThrow("'use strict'; delete aDeletableProperty;");
shouldThrow("'use strict'; (function (){ delete someDeclaredGlobal;})");
shouldThrow("(function (){ 'use strict'; delete someDeclaredGlobal;})");
shouldBeTrue("'use strict'; if (0) { someGlobal = 'Shouldn\\'t be able to assign this.'; }; true;");
shouldThrow("'use strict'; someGlobal = 'Shouldn\\'t be able to assign this.'; ");
shouldThrow("'use strict'; eval('var introducedVariable = \"FAIL: variable introduced into containing scope\";'); introducedVariable");
var objectWithReadonlyProperty = {};
Object.defineProperty(objectWithReadonlyProperty, "prop", {value: "value", writable:false});
shouldThrow("'use strict'; objectWithReadonlyProperty.prop = 'fail'");
shouldThrow("'use strict'; delete objectWithReadonlyProperty.prop");
readonlyPropName = "prop";
shouldThrow("'use strict'; delete objectWithReadonlyProperty[readonlyPropName]");
shouldThrow("'use strict'; ++eval");
shouldThrow("'use strict'; eval++");
shouldThrow("'use strict'; --eval");
shouldThrow("'use strict'; eval--");
shouldThrow("'use strict'; function f() { ++arguments }");
shouldThrow("'use strict'; function f() { arguments++ }");
shouldThrow("'use strict'; function f() { --arguments }");
shouldThrow("'use strict'; function f() { arguments-- }");
var global = this;
shouldBeTrue("global.eval('\"use strict\"; if (0) ++arguments; true;')");
shouldThrow("'use strict'; ++(1, eval)");
shouldThrow("'use strict'; (1, eval)++");
shouldThrow("'use strict'; --(1, eval)");
shouldThrow("'use strict'; (1, eval)--");
shouldThrow("'use strict'; function f() { ++(1, arguments) }");
shouldThrow("'use strict'; function f() { (1, arguments)++ }");
shouldThrow("'use strict'; function f() { --(1, arguments) }");
shouldThrow("'use strict'; function f() { (1, arguments)-- }");
shouldThrow("'use strict'; if (0) delete +a.b");
shouldThrow("'use strict'; if (0) delete ++a.b");
shouldThrow("'use strict'; if (0) delete void a.b");

shouldBeTrue("(function (a){'use strict'; a = false; return a !== arguments[0]; })(true)");
shouldBeTrue("(function (a){'use strict'; arguments[0] = false; return a !== arguments[0]; })(true)");
shouldBeTrue("(function (a){'use strict'; a=false; return arguments; })(true)[0]");
shouldBeTrue("(function (a){'use strict'; arguments[0]=false; return a; })(true)");
shouldBeTrue("(function (a){'use strict'; arguments[0]=true; return arguments; })(false)[0]");
shouldBeTrue("(function (){'use strict';  arguments[0]=true; return arguments; })(false)[0]");
shouldBeTrue("(function (a){'use strict'; arguments[0]=true; a=false; return arguments; })()[0]");
shouldBeTrue("(function (a){'use strict'; arguments[0]=false; a=true; return a; })()");
shouldBeTrue("(function (a){'use strict'; arguments[0]=true; return arguments; })()[0]");
shouldBeTrue("(function (){'use strict';  arguments[0]=true; return arguments; })()[0]");

// Same tests again, this time forcing an activation to be created as well
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); a = false; return a !== arguments[0]; })(true)");
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); arguments[0] = false; return a !== arguments[0]; })(true)");
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); a=false; return arguments; })(true)[0]");
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); arguments[0]=false; return a; })(true)");
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); arguments[0]=true; return arguments; })(false)[0]");
shouldBeTrue("(function (){'use strict';  var local; (function (){local;})(); arguments[0]=true; return arguments; })(false)[0]");
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); arguments[0]=true; a=false; return arguments; })()[0]");
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); arguments[0]=true; return arguments; })()[0]");
shouldBeTrue("(function (a){'use strict'; var local; (function (){local;})(); arguments[0]=false; a=true; return a; })()");
shouldBeTrue("(function (){'use strict';  var local; (function (){local;})(); arguments[0]=true; return arguments; })()[0]");

shouldBeTrue("'use strict'; (function (){var a = true; eval('var a = false'); return a; })()");
shouldBeTrue("(function (){var a = true; eval('\"use strict\"; var a = false'); return a; })()");

shouldBeUndefined("(function f(arg){'use strict'; return Object.getOwnPropertyDescriptor(f, 'arguments').value; })()");
shouldBeUndefined("(function f(arg){'use strict'; return Object.getOwnPropertyDescriptor(f, 'caller').value; })()");
shouldBeUndefined("(function f(arg){'use strict'; return Object.getOwnPropertyDescriptor(arguments, 'callee').value; })()");
shouldBeUndefined("(function f(arg){'use strict'; return Object.getOwnPropertyDescriptor(arguments, 'caller').value; })()");
shouldBeTrue("(function f(arg){'use strict'; var descriptor = Object.getOwnPropertyDescriptor(arguments, 'caller'); return descriptor.get === descriptor.set; })()");
shouldBeTrue("(function f(arg){'use strict'; var descriptor = Object.getOwnPropertyDescriptor(arguments, 'callee'); return descriptor.get === descriptor.set; })()");
shouldBeTrue("(function f(arg){'use strict'; var descriptor = Object.getOwnPropertyDescriptor(f, 'caller'); return descriptor.get === descriptor.set; })()");
shouldBeTrue("(function f(arg){'use strict'; var descriptor = Object.getOwnPropertyDescriptor(f, 'arguments'); return descriptor.get === descriptor.set; })()");
shouldBeTrue("'use strict'; (function f() { for(var i in this); })(); true;")

var successfullyParsed = true;
