-begin
-i
@pragma set_user_agent(msie 7.0);
-o
-end

-begin
-i
@pragma set_user_agent(msie 7.0);

#a {
    color: red;
    color(msie 7.0): green;
    color(msie 6.0): blue;
}
-o
#a{color:green;}
-end

-begin
-i
@pragma set_user_agent(msie);

#a {
    color(msie 7.0): green;
    color(msie 6.0): blue;
}

#b {
    color(msie): yellow;
    color: red;
}

#c {
    color(msie): blue;
    color(msie 5.0): yellow;
}

#d {
    color(firefox): purple;
}
-o
#a{color:blue;}
#b{color:red;}
#c{color:yellow;}
-end

-begin
-i
@pragma set_user_agent(msie 5.5);

#a {
    color(msie): red;
}
-o
#a{color:red;}
-end
