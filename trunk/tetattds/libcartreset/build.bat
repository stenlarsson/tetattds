rename Makefile.arm7 Makefile
make clean
del lib\*.* /Q
rmdir lib
make
mkdir lib
cp .\build\libcartreset7.a .\lib\libcartreset7.a
rename Makefile Makefile.arm7

rename Makefile.arm9 Makefile
make clean
make
cp .\build\libcartreset9.a .\lib\libcartreset9.a
rename Makefile Makefile.arm9

del build\*.* /Q
rmdir build
pause