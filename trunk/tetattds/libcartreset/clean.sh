mv Makefile.arm7 Makefile
make clean
mv Makefile Makefile.arm7

mv Makefile.arm9 Makefile
make clean
mv Makefile Makefile.arm9

rm -rf lib/

rm -rf build/