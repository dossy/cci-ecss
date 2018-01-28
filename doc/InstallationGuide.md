  * [Ubuntu](#ubuntu)
    * [Ubuntu prerequisites](#ubuntu-prerequisites)
    * [Installing the Ubuntu deb packages](#installing-the-ubuntu-deb-packages)
    * [Enabling mod_css on Ubuntu](#enabling-mod_css-on-ubuntu)
  * [Building from source](#building-from-source)
    * [Quick start: UNIX](#quick-start-unix)
    * [Source prerequisites](#source-prerequisites)
    * [Building mod_ecss (the ecss apache module)](#building-mod_ecss)
  * [Configuring mod_ecss](#configuring-mod_ecss)
    * [Enabling mod_ecss](#enabling-mod_ecss)
    * [ecss apache directives](#ecss-apache-directives)
    * [Alternatives to mod_ecss](#alternatives-to-mod_ecss)

## Ubuntu

These instructions have only been tested on Ubuntu (Hardy) but should work on other Debian-based systems.

### Ubuntu prerequisites

The ecss command line executable depends on the libboost-program-options package. If you're not interested in using ecss on the command line, you can skip this step.

```
apt-get install libboost-program-options
```

### Installing the Ubuntu deb packages

Whether you want to install ecss on the command line or as an apache module, you'll need the libecss package: [libecss_1.0-1_i386.deb](/dossy/cci-ecss/blob/code_google_com/libecss_1.0-1_i386.deb). To install the deb, either double-click on it or use dpkg:

```
dpkg --install libecss_1.0-1_i386.deb
```

Next, you'll need to download either the ecss executable package ([ecss-bin_1.0-1_i386.deb](/dossy/cci-ecss/blob/code_google_com/ecss-bin_1.0-1_i386.deb)) or the ecss apache2 module ([libapache2-mod-ecss_1.0-1_i386.deb](/dossy/cci-ecss/blob/code_google_com/libapache2-mod-ecss_1.0-1_i386.deb)) or both, and install them the same way:

```
dpkg --install ecss-bin_1.0-1_i386.deb
dpkg --install libapache2-mod-ecss_1.0-1_i386.deb
```

### Enabling mod_css on Ubuntu

Run a2enmod and follow its instructions (it will probably instruct you to restart or reload apache):

```
a2enmod ecss
```

## Building from source

### Quick start UNIX

```
./configure --prefix=PREFIX
make test
make install
```

NOTES:
  * You will need the boost C++ library installed on your system in order to compile ecss or use the command-line compiler. See [Source prerequisites](#source-prerequisites) for more information.
  * If you don't specify a prefix, it will default to `/usr/local`.
  * If you install ecss into a non-standard directory, you will have to either (a) set your `LD_LIBRARY_PATH` environment variable to include `PREFIX/lib` or (b) add `PREFIX/lib` to your `/etc/ld.so.conf` file (and run `ldconfig`).

### Source prerequisites

You will need the boost C++ library installed on your system. ecss requires only one non header-only library: lib_program_options. On Debian and Ubuntu, you can install boost with the following commands:

```
apt-get install libboost-dev
apt-get install libboost-program-options-dev
```

On Fedora and RH Enterprise:

```
yum install boost-devel
```

Alternatively, you can download boost (http://www.boost.org/) and build and install it by hand. Note: If you do not install boost in the default location, you will have to pass the `--with-boost-headers` and `--with-boost-lib` options to ecss's `configure`.

### Building mod_ecss

First, build ecss (see above) and make sure the shared library (typically named `libecss.so`) is installed somewhere it can be found by the operating system. Then:

```
cd src/mod_ecss
./configure --with-ecss=PREFIX --with-apache=/path/to/apache
make
make install
```

NOTES:
  * For `--with-ecss`, use the same `PREFIX` you passed to ecss's `configure` program. If you did not specify a prefix when building ecss, use `/usr/local` here.
  * `configure` will look for a program named `bin/apxs` underneath the directory passed to `--with-apache`.
  * Only tested with Apache 2.x

## Configuring mod_ecss

### Enabling mod_ecss

(Also see [Enabling mod_css on Ubuntu](#enabling-mod_css-on-ubuntu).)

Add the following lines to your `httpd.conf` file:

```
LoadModule ecss_module modules/ecss.so
<IfModule ecss_module>
AddOutputFilter ECSS ecss
</IfModule>
```

That will cause files ending with "`.ecss`" to be passed through the ecss compiler.

### ecss apache directives

You can set the ecss include path (see [IncludeFiles](IncludeFiles.md)) with the `ECSS_SetIncludePath` directive. For example:

```
<IfModule ecss_module>
ECSS_SetIncludePath "/home/dcowgill/public_html/ecss:/www/src/projectx/ecss"
AddOutputFilter ECSS ecss
</IfModule>
```

You can specify whether ruleset declaration order should be preserved (see [Gotchas](Gotchas.md)) with the `ECSS_PreserveOrder` directive. For example:

```
<IfModule ecss_module>
ECSS_PreserveOrder On
AddOutputFilter ECSS ecss
</IfModule>
```

### Alternatives to mod_ecss

Use [FastCGI](http://www.fastcgi.com/) with the ecss command-line compiler.
