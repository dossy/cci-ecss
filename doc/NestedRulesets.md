ecss allows the author to nest rulesets; a child ruleset combines the selector of its parent with its own selector. For example:

```
.a {
    width: 100px;
    .b {
        height: 50px;
        .c {
            text-align: center;
        }
    }
}
```

Is translated into the following CSS:

```
.a{width:100px;}
.a .b{height:50px;}
.a .b .c{text-align:center;}
```

## Unary Combinators

Although it may not be obvious from the above example, selectors in nested rulesets must begin with a unary combinator. If no combinator is given, the descendant combinator (which is represented syntactically by whitespace) is used implicitly. But it is possible to use any of the following combinators:
  * <whitespace> (descendant of)
  * `>` (child of)
  * `+` (adjacent sibling of)
  * `~` (general sibling of)

For example, this group of nested selectors uses every one of the unary combinators:

```
.a {
    .b {
        > .c {
            + .d {
                ~ .e {
                    hello: world;
                }
            }
        }
    }
}
```

The result:

```
.a .b > .c + .d ~ .e{hello:world;}
```
