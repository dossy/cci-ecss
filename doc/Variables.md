You may declare a variable wherever you could declare a top-level ruleset. As a general rule, you can use variables on the righthand side of any ":" expression. For example:

```
$width: thin;
$style: solid;
$color: red;
$thin_red_line: $width $style $color;
div.war { border: $thin_red_line; }
```

The compiled result:

```
div.war { border: thin solid red; }
```

It is illegal, however, to use a variable on the lefthand side of an expression:

```
$property_name: color;
div {
    $property_name: red;        /* error: can't use in the lefthand side of a property */
}
```

Note that the leading dollar sign in variable names is required.

## Readonly

As a precaution against the accidental overwriting of a variable, you may indicate, when the variable is declared, that it cannot be redefined:

```
$immutable readonly: hello;
$mutable: world;
$immutable readonly: goodbye;   /* error: can't change a readonly variable */
$immutable: goodbye;            /* error: already defined as readonly */
$mutable readonly: world;       /* error: already defined as non-readonly */
```
