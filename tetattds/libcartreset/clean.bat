rename Makefile.arm7 Makefile
make clean
rename Makefile Makefile.arm7

rename Makefile.arm9 Makefile
make clean
rename Makefile Makefile.arm9

del lib\*.* /Q
rmdir lib

del build\*.* /Q
rmdir build

pause