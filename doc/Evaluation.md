The `eval` function provides access to a very simple infix calculator; it also supports string concatenation. Here is a summary of the operators and expressions:

| | |
| :---: | --- |
|  `+` | add, or identity in a unary context |
|  `-` | subtract, or negate in a unary context |
|  `*` | multiply |
|  `/` | divide |
|  `_` | concatenate as text |
|  `(...)` | sub-expression |
|  `floor(...)` | round down to nearest integer |

Note that unlike most C-family languages, the `eval` calculator does not assign different precedence to its operators. Thus, `4+10/2` is equivalent to `(4+10)/2` not `4+(10/2)`.

Here's a real-world example:

```
$width: 800;
$width_px: eval($width _ px);
$horiz_padding: eval(floor($width * 0.018));
$horiz_padding_px: eval($horiz_padding _ px);
$vertical_padding: eval(floor($horiz_padding / 6));
$vertical_padding_px: eval($vertical_padding _ px);
$base_font_size: $horiz_padding;
$base_font_size_px: eval($base_font_size _ px);
$border_width: 1;
$border_width_px: eval($border_width _ px);
$content_width: eval(floor($width * .7));
$hilite_color: #6a8bb8;
$soft_border_color: #ccc;
$strong_border_color: #000;
```
