-begin
-i
#test { em { color: red } }
-o
#test em{color:red;}
-end


-begin
-i
#test {
    a:0;
    #b {
        b:0;
        #c {
            c:0;
            c:1;
        }
        b:1;
    }
    a:1;
}
-o
#test{a:1;}
#test #b{b:1;}
#test #b #c{c:1;}
-end

-begin
-i
div#test {
    #xyz {
        font-size: 12pt;
    }
    color: white;
}
div#test {
    background-color: red;
}
div#test #xyz {
    font-weight: bold;
}
-o
div#test{color:white;background-color:red;}
div#test #xyz{font-size:12pt;font-weight:bold;}
-end

-begin
-i
#test #a {
    a: b;
    x: y;
    x: z;
    a: c;
}

#test #b {
    a: b;
}

#test #b {
    x: y;
}

#test #b {
    x: z;
}

#test #b {
    a: c;
}
-o
#test #a{x:z;a:c;}
#test #b{x:z;a:c;}
-end

-begin
-i
$x: 12pt;
#test div { font-size: $x }
-o
#test div{font-size:12pt;}
-end

-begin
-i
#test .a > .b + .c ~ .d { aaa:111 }

#test .a {
  > .b + .c ~ .d {
    bbb:222
  }
}

#test .a > .b {
  + .c ~ .d {
    ccc:333
  }
}

#test .a > .b + .c {
  ~ .d {
    ddd:444
  }
}

#test .a {
  > .b {
    + .c {
      ~ .d {
        eee:555
      }
    }
  }
}
-o
#test .a > .b + .c ~ .d{aaa:111;bbb:222;ccc:333;ddd:444;eee:555;}
-end

-begin
-i
#test.abc {
    height: 200px;
    width: 100px !important;
}
-o
#test.abc{height:200px;width:100px !important;}
-end

-begin
-i
#test foo {
    background:
        url(/images/icons/blah.gif)
        no-repeat
        0
        -229px;
}
-o
#test foo{background:url(/images/icons/blah.gif) no-repeat 0 -229px;}
-end

-begin
-i
#test.a {
    foo: bar(baz);
}
-o
#test.a{foo:bar(baz);}
-end

-begin
-i
div {
    color: $oops;
}
-o
ecss: <>:2:11: variable $oops not defined
-end

-begin
-i
a
,
b
{
x:y
}
-o
a{x:y;}
b{x:y;}
-end

-begin
-i
span.foo.bar.baz, div.a.b.c ul.d.e.f {
    color: black;
}
-o
div.a.b.c ul.d.e.f{color:black;}
span.foo.bar.baz{color:black;}
-end

-begin
-i
div.foo {
    width: 200px;
    width: 300px ! important;
    width: 400px;
}
div.foo div.bar {
    width: 500px !important;
    width: 600px;
    width: 700px !important;
    width: 800px;
}
-o
div.foo{width:300px !important;}
div.foo div.bar{width:700px !important;}
-end

-begin
-i
div[name=foo] {
    width: 123px;
}
div[xmlns] a:hover {
    color: red;
}
div[class=bar] .baz {
    text-align: center;
}
-o
div[class=bar] .baz{text-align:center;}
div[name=foo]{width:123px;}
div[xmlns] a:hover{color:red;}
-end

-begin
-i
$x readonly: 1;
$x: 2;
-o
ecss: <>:2:0: variable $x already defined readonly at <>:1:0
-end

-begin
-i
$x readonly: 1;
$x readonly: 2;
-o
ecss: <>:2:0: variable $x already defined readonly at <>:1:0
-end

-begin
-i
$x: 1;
$x readonly: 2;
-o
ecss: <>:2:0: variable $x already defined non-readonly at <>:1:0
-end

-begin
-i
.c { width: 1px; }
.b { width: 2px; }
.a { width: 3px; }
-o
.a{width:3px;}
.b{width:2px;}
.c{width:1px;}
-end

-begin
-i
@pragma preserve_order();
.c { width: 1px; }
.b { width: 2px; }
.a { width: 3px; }
-o
.c{width:1px;}
.b{width:2px;}
.a{width:3px;}
-end

-begin
-i
.a { color: red; }
.b { color: white; }
.a { color: blue; }
-o
.a{color:blue;}
.b{color:white;}
-end

-begin
-i
@pragma preserve_order();
.a { color: red; }
.b { color: white; }
.a { color: blue; }
-o
.a{color:red;}
.b{color:white;}
.a{color:blue;}
-end
