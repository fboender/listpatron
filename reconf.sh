#!/bin/sh
#
# Custom script
#
aclocal && autoheader && automake --add-missing --copy && autoconf
