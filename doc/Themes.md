A theme is a named list of properties and rulesets. Once defined, a theme can be reused within a ruleset or another theme. The syntax for a theme definition is as follows: `'@define-theme' theme-name '{' property-or-ruleset+ '}'`

For example:

```
@define-theme my-theme {
    div {
        color: red;
        span { font-size: small }
    }
}
```

## A small example

```
@define-theme personal-pages {
    color: dark-green;
    background-color: dark-red;
    marquee: yes;
    blink: frequently;
    a {
        color: #000;
    }
}
div#main-body {
    @theme personal-pages;
    font-size: huge;
}
```

The result:

```
div#main-body{color:dark-green;background-color:dark-red;marquee:yes;blink:frequently;font-size:huge;}
div#main-body a{color:#000;}
```

## Defining themes

A theme may be defined anywhere at the top level of a source file. They may not be nested within rulesets or other themes.

```
@define-theme foo { a:b }          /* ok */
@define-theme bar {
    @define-theme blah { a:b }     /* error: cannot define a theme within a theme */
}
div {
    @define-theme blah { a:b }     /* error: cannot define a theme within a ruleset */
}
```

It is also an error to define a theme with the same name twice:

```
@define-theme foo { a:b }          /* ok */
@define-theme foo { a:b }          /* error: theme "foo" already defined */
```

## Using themes

A theme may be used anywhere a property could be declared. Note that a theme definition must precede its use:

```
div { @theme foo; }                /* error: theme "foo" not defined */
@define-theme foo { color:black }
@theme foo;                        /* error: a theme must be used within a ruleset */
div {
    @theme foo;
    color: @theme foo;             /* error: a theme can only be used where a property declaration would appear */
    span { @theme foo; }
}
```

## Composition

Note that themes may be composed, i.e. you can use a theme within the definition of another theme. This provides a de facto form of multiple inheritance:

```
@define-theme a {
    color: red;
    span.x { a { color: blue; } }
}
@define-theme b {
    @theme a;
    span.x { a { font-size: 12pt; } }
}
div {
    @theme b;
}
```

The result:

```
div{color:red;}
div span.x a{color:blue;font-size:12pt;}
```
