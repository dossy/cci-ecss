You can include one ecss source file within another using the @include statement, which expects either a url or a string specifying the path to the file. For example:

```
@include url(a.ecss);
@include "/tmp/b.css";
@include url(../lib/c.inc);
@include "x/y/z/foo.bar";
```

Note that the path given to `@include` may be absolute or relative. For more information about how relative paths are resolved, see [The include path](#the-include-path) below.

The `@include` statement may only appear at the top level of a source file; it cannot be nested within rulesets, theme definitions, and so on. 

## The include path

When you give `@include` a relative path, the compiler first searches its include path (which, despite its name, is in fact an ordered list of directories) for the file. If the compiler is unable to find the file in any of the directories in the include path, it will search the including file's directory before finally giving up.

Typically, the include path is set via a command line option (when using the compiler on the command line) or an Apache directive (when using it in a web server), but for testing purposes you can also set the include path using the `set_include_path` pragma, as in the following example:

```
@pragma set_include_path("/tmp/foo:/usr/local:/home/bar/baz");
@include "x/y/z.inc";
```

If the above source code were stored in a file named "`/a/b/c.ecss`", the following paths would be searched for the included file, in order:

  * `/tmp/foo/x/y/z.inc`
  * `/usr/local/x/y/z.inc`
  * `/home/bar/baz/x/y/z.inc`
  * `/a/b/x/y/z.inc`

## Redundant includes

As a convenience, if you include the same file more than once, the compiler will ignore the second and subsequent `@include` statements. 
