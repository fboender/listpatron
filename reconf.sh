#!/bin/sh
#
# Custom script
#
# This script prepares a cvs checkout of ListPatron for development.
# It reruns various autotools, creates some symlinks and configures the
# sources so you can start developing in the src/ dir.
#
# Also run this tool whenever you make changes to any of the .am or .in 
# files.
#

#!/bin/sh

echo "Running autotools.."
aclocal && autoheader && automake --add-missing --copy && autoconf
echo "Creating synthetic /usr/share in current dir.."
rm pixmaps/listpatron
ln -s . pixmaps/listpatron
rm xml/listpatron
ln -s . xml/listpatron
echo "Configuring the source (debugging enabled).."
./configure --datadir=`pwd` --enable-debug
