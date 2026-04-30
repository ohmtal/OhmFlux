# !! must be called ./createIcons.sh !!
# ANDROIDPATH=./app/src/main/res/

python ../../Tools/make_android_icons.py logo.png ./app/src/main/res/
python ../../Tools/make_windows_icon.py logo.png ./icon.ico
