-begin
-i
div { color: /*red*/blue; }
/*.a { x:y }*/
.a { x:z }
-o
.a{x:z;}
div{color:blue;}
-end

-begin
-i
div {
    /*
    a:b;
}
-o
ecss: <>:0:-1: Encountered end of file when expecting one of the following: VARIABLE IDENT STAR CLASS HASH LBRACKET COLON
-end
