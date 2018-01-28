-begin
-i
@define-theme foo { a: b }
div { @theme foo }
-o
div{a:b;}
-end

-begin
-i
@define-theme foo {
    a: b;
}

@define-theme bar {
    x: y;
    @theme foo;
}

div {
    @theme bar;
}
-o
div{x:y;a:b;}
-end

-begin
-i
@define-theme foo {
    a: b;
    .a {
        w: x;
        .b {
            y: z;
        }
    }
    .c {
        d: e;
    }
}

.x {
    @theme foo;
    .a {
        w: xx;
    }
    .a .b {
        y: zz;
    }
    .c {
        d: ee;
    }
}

.y {
    .a {
        w: xx;
    }
    .a .b {
        y: zz;
    }
    .c {
        d: ee;
    }
    @theme foo;
}

.z {
   @theme foo;
}

.w {
    div {
        span {
            @theme foo;
        }
        @theme foo;
    }
}
-o
.w div{a:b;}
.w div .a{w:x;}
.w div .a .b{y:z;}
.w div .c{d:e;}
.w div span{a:b;}
.w div span .a{w:x;}
.w div span .a .b{y:z;}
.w div span .c{d:e;}
.x{a:b;}
.x .a{w:xx;}
.x .a .b{y:zz;}
.x .c{d:ee;}
.y{a:b;}
.y .a{w:x;}
.y .a .b{y:z;}
.y .c{d:e;}
.z{a:b;}
.z .a{w:x;}
.z .a .b{y:z;}
.z .c{d:e;}
-end

-begin
-i
div { @theme not-defined; }
-o
ecss: <>:1:6: theme "not-defined" not defined
-end

-begin
-i
@define-theme foo {
    @define-theme bar {
        a:b;
    }
}
-o
ecss: <>:2:4: Encountered the symbol "@define-theme" when expecting one of the following: VARIABLE IDENT STAR CLASS HASH LBRACKET COLON
-end

-begin
-i
@define-theme foo {
    a:b;
}

@define-theme foo {
    a:b;
}
-o
ecss: <>:5:0: theme "foo" already defined at <>:1:0
-end

-begin
-i
div {
    @define-theme foo { a:b }
}
-o
ecss: <>:2:4: Encountered the symbol "@define-theme" when expecting one of the following: VARIABLE IDENT STAR CLASS HASH LBRACKET COLON
-end
