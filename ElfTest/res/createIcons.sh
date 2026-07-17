# !! must be called ./createIcons.sh !!
# ANDROIDPATH=./app/src/main/res/

python ../../Tools/make_android_icons.py icon.png ./app/src/main/res/
python ../../Tools/make_windows_icon.py icon.png ./icon.ico
