You may associate a property with a specific user agent by placing a parenthesized user agent string after the property name. For example:

```
.a {
    color           : red;
    color(msie)     : orange;
    color(msie 5.5) : yellow;
    color(msie 6.0) : green;
    color(msie 7.0) : blue;
    color(mozilla)  : indigo;
    color(opera)    : violet;
}
```

When the user agent is MSIE 6.0, ecss will compile the above ruleset into the following CSS code:

```
.a{color:red;color:orange;color:green;}
```

...which ecss will ultimately reduce to

```
.a{color:green;}
```

Note that "`color: red`" will match any user agent. It must therefore precede the more specific properties, which may override it. Likewise, "`color(msie): orange`" must precede all of the msie-only properties that also have version numbers.

## Syntax

```
property : property-name ':' property-value ';'
         | property-name '(' user-agent-name ')' ':' property-value ';'
         | property-name '(' user-agent-name version ')' ':' property-value ';'
```

## Supported user agents

Here's a list of the user agents that the ecss compiler recognizes by default, along with the regular expression used to match the user agent.

| | |
| --- | --- |
| `msie 5.0` | `^mozilla.*msie 5[.]0` |
| `msie 5.5` | `^mozilla.*msie 5[.]5` |
| `msie 6.0` | `^mozilla.*msie 6[.]0` |
| `msie 7.0` | `^mozilla.*msie 7[.]0` |
| `msie` | `^mozilla.*msie` |
| `mozilla` | `^mozilla.*gecko` |
| `safari` | `^mozilla.*apple.*safari` |
| `lynx` | `^lynx` |
| `opera` | `opera` |

## Limitations

User agent-specific properties won't work if you compile ecss code outside the web server. You must use the apache module `mod_ecss` instead.
