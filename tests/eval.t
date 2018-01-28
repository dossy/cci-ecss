-begin
-i
$font-size: eval((1+2) * (3+1) _ pt);
div { font-size: $font-size; }
-o
div{font-size:12pt;}
-end

-begin
-i
$five-factorial: eval(1*2*3*4*5);
$unit: eval(p _ t);
.t { font-size: eval($five-factorial _ $unit); }
-o
.t{font-size:120pt;}
-end

-begin
-i
.t { x: eval(4 + 80 / 2); }
-o
.t{x:42;}
-end

-begin
-i
#x {
    a: eval(1+2, 3+4, 5+6);
}
-o
ecss: <>:2:15: illegal operator ',' in eval
-end

-begin
-i
a {
    b: eval(1=2);
}
-o
ecss: <>:2:13: illegal operator '=' in eval
-end

-begin
-i
$a: 3.14159;
$x: eval($a * $a);
$y: eval($a * $b);
-o
ecss: <>:3:14: variable $b not defined
-end

-begin
-i
div {
    font-size: eval(10*$x _ pt);
}
-o
ecss: <>:2:23: variable $x not defined
-end

-begin
-i
$a: 13;
$b: eval($a/2);
$c: eval(floor($a/2));
$d: eval(floor(1.5*$a) + floor(1.5*$a));
div {
    a: $a;
    b: $b;
    c: $c;
    d: $d;
}
-o
div{a:13;b:6.5;c:6;d:38;}
-end

-begin
-i
div {
    width: eval(foo(42));
}
-o
ecss: <>:2:16: illegal function 'foo' in eval
-end
