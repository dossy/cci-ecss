**ecss** is a fully compatible CSS variant. **mod\_ecss** (the ecss apache module) has been in production use since May 2007 serving millions of requests per day. Besides syntax-checking and compressing existing CSS, ecss provides the following extensions:

  * [NestedRulesets](doc/NestedRulesets.md)
  * [Variables](doc/Variables.md)
  * [Themes](doc/Themes.md)
  * [IncludeFiles](doc/IncludeFiles.md)
  * [Evaluation](doc/Evaluation.md)
  * [UserAgentProperties](doc/UserAgentProperties.md)

Here's a brief example illustrating some of ecss's features:

```
@include "other-stuff/ads.ecss";

$width: 800;
$width_px: eval($width _ px);
$horiz_padding: eval(floor($width * 0.018));
$horiz_padding_px: eval($horiz_padding _ px);
$vertical_padding: eval(floor($horiz_padding / 6));
$vertical_padding_px: eval($vertical_padding _ px);

#myproject {
  #menu {
    ul, ol {
      list-style: none;
      li {
        height: eval($height _ em);
        padding: 0;
      }
      a {
        color: #aabbcc;
        color(msie 7.0): #ddeeff;
      }
    }
  }
}
```

More stuff to read:

  * [InstallationGuide](doc/InstallationGuide.md)
  * [Gotchas](doc/Gotchas.md)
  * [History](doc/History.md)
  * [LICENSE](LICENSE)
