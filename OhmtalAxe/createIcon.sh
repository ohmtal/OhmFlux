#!/bin/sh
# Version: XXTH 2025-11-11
#
# * create and install an Menu-Entry (Icon) for your Application
# * create a removeIcon.sh script to remove it again

# your Application variables:
# APPNAME: blanks in your APPNAME are replaced with underscore "_".
APPNAME="Axe"
EXE="OhmtalAxe.x86_64"
ADDCURRENTPATHTOEXE="1"
VERSION="1.2602"
COMMENT="Guitar Effect Machine"
# ICON: relative path to this script.
ICON="assets/icon.bmp"
# CATEGORIES: multipe are separated with semicolons example: Game;Simulation
# if you are not sure which to choose look at the .desktop file in:
# /usr/share/applications
# or even better at: https://specifications.freedesktop.org/menu-spec/1.0/category-registry.html
CATEGORIES="Audio"
#------------------------------------------------------------------------------
# move to current directory
CURPATH=`dirname "$0"`
cd $CURPATH
GAMEPATH=`pwd`
DESKTOP_MODE="user"
ICON_SIZE="48"
DESKTOPFILE=`echo $APPNAME| tr ' ' '_'`.desktop


FILENAME="$GAMEPATH/$DESKTOPFILE"
ICONFILE="$GAMEPATH/$ICON"

MENUCMD=`command -v xdg-desktop-menu`
if [ "`command -v xdg-user-dir`" == "" ]; then
    DESKTOPPATH=$( eval echo "~/Desktop/" )
else
    DESKTOPPATH=`xdg-user-dir DESKTOP`
fi


# create desktop file:
echo "[Desktop Entry]" > $FILENAME
echo "Version=$VERSION" >> $FILENAME
echo "Type=Application" >> $FILENAME
echo "Name=$APPNAME" >> $FILENAME
echo "Comment=$COMMENT" >> $FILENAME
if [ "$ADDCURRENTPATHTOEXE" = "1" ]; then
    echo "Exec=$GAMEPATH/$EXE" >> $FILENAME
else
    echo "Exec=$EXE" >> $FILENAME
fi
echo "Icon=$ICONFILE" >> $FILENAME
echo "Path=$GAMEPATH" >> $FILENAME
echo "Terminal=false" >> $FILENAME
echo "Categories=$CATEGORIES" >> $FILENAME
echo "StartupNotify=false" >> $FILENAME


# check if we can do anything
if [ ! -d $DESKTOPPATH ] && [ "$MENUCMD" = "" ]; then
    echo "---------------------------------------"
    echo "CreateIcon failed!"
    echo "! Command xdg-desktop-menu not found. "
    echo "! $DESKTOPPATH does not exists."
    echo "* but $FILENAME created, you can copy it manually on your Desktop."
    echo "---------------------------------------"
    exit 1
fi

#register in menu

if [ "$MENUCMD" != "" ]; then
    $MENUCMD install --novendor --mode $DESKTOP_MODE $FILENAME
fi

# add to desktop
if [ -d $DESKTOPPATH ]; then
  cp $FILENAME $DESKTOPPATH
fi


#write removeIcon
REMOVEICONSCRIPT="$CURPATH/removeIcon.sh"
echo "#!/bin/sh" > $REMOVEICONSCRIPT
if [ "$MENUCMD" != "" ]; then
    echo "$MENUCMD uninstall --novendor --mode $DESKTOP_MODE $DESKTOPFILE" >> $REMOVEICONSCRIPT
fi
if [ -d $DESKTOPPATH ]; then
    echo "rm $DESKTOPPATH/$DESKTOPFILE" >> $REMOVEICONSCRIPT
fi
chmod 755 $REMOVEICONSCRIPT

echo "---------------------------------------"
echo "CreateIcon finished..."
echo "* desktop file: $FILENAME created"
if [ "$MENUCMD" != "" ]; then
    echo "* added $DESKTOPFILE to your menu in Categories: $CATEGORIES"
fi
if [ -d $DESKTOPPATH ]; then
    echo "* added $DESKTOPFILE to your Desktop ($DESKTOPPATH)"
fi
echo "* created $REMOVEICONSCRIPT, if you want to remove the icon"
echo "---------------------------------------"
