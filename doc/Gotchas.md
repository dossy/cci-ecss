## Specificity and declaration order

The most important difference between ecss and CSS is that ecss does not respect the order in which rulesets are declared. For example, in CSS the following rulesets have equal specificity:

```
ul li         {}  /* a=0 b=0 c=0 d=2 -> specificity = 0,0,0,2 */
li:first-line {}  /* a=0 b=0 c=0 d=2 -> specificity = 0,0,0,2 */
```

As a result, the order in which they appear is significant. If equally specific rulesets contain conflicting properties, the final one takes precedence. For example, given the following CSS:

```
.a { color: red; }
.b { color: blue; }
```

...whether `<div class="a b">...</div>` is red or blue depends on the order in which the rulesets appear in the source code. Unfortunately, this misfeature of CSS makes style sheets more fragile and harder to maintain. Well-written CSS does not rely on declaration order.

By default, ecss reserves the right to reorder rulesets at will; it does this in order to more effectively optimize its final output. But this behavior can be changed by passing the `--preserve-order` (or `-p`) flag to the command-line ecss compiler and by using the `ECSS_PreserveOrder` apache directive (see [InstallationGuide](doc/InstallationGuide.md)).

## LVHA (aka :link, :hover, :visited, :active)

Styling links with the `:link`, `:visited`, `:hover`, and `:active` pseudo-classes is extraordinarily tricky in CSS, and ecss's tendency to reorder declarations can make it even trickier. Fortunately, adhering to existing best practices for styling links addresses most of the problems.

| | |
| --- | --- |
| To match visited links | `:visited` |
| To match unvisited links | `:link` |
| To match hover links | `:link:hover, :visited:hover` |
| To match active links | `:link:active, :visited:active` |
| To match focus links | `:link:focus, :visited:focus` |

As long as you follow the above guidelines (which are recommended even when not using ecss), your CSS should work as expected in any modern browser even if ecss decides to reorder your rulesets. There is one caveat, however: hover links _may_ override the style for active links. To make sure the active link style takes precedence, use the following selector: `a:link:active:hover, a:visited:active:hover`

## expression (Internet Explorer only)

ecss does not support the IE-specific "expression" extension. This is fortunate for users of ecss because expressions [should be universally avoided](http://developer.yahoo.com/performance/rules.html#css_expressions).

ecss provides a [better solution for one-time computations](Evaluation.md). CSS changes that truly need to be recomputed on the fly are best implemented natively in JavaScript.

## The "Box Model Hack"

This well-known MSIE workaround relies on a bug present in the MSIE and Opera CSS parsers:

```
#elem { 
  width: 150px; 
  voice-family: "\"}\"";
  voice-family:inherit;
  width: 120px;
}
html>body #elem {
  width: 120px;
} 
```

This hack won't work with ecss, which dutifully compiles the above into

```
#elem{voice-family:inherit;width:120px;}
html > body #elem{width:120px;}
```

Fortunately, ecss provides a more elegant solution in [user agent-specific properties](UserAgentProperties.md). For example:

```
#elem { 
  width: 120px;
  width(msie): 150px; 
}
```
