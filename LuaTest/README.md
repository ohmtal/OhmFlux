# LuaTest

---

### LUA Compile and Linking:

I finally added lua and sol2 to include, compile Lua as
c++ to fix the linker errors. I also generate a lua.hpp
which include the files directly without "extern C"
There are some deprecated messages ... I will go over
this bridge when it's fail.

---

### Why did I change it:
In my previous CMakeList i thought my custom compiled
lua is used but it was not. After the system lua was
updated to 5.5x the problems began. Took me some hours
to get sol2 using the correct headers - i guess the
problem was that in the lua sources not lua.hpp exists.
After getting over this the Linker failed. So i changed
it so CXX and added a lua.hpp.
    
