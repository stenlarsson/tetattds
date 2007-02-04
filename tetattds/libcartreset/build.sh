mv Makefile.arm7 Makefile
make clean
rm -rf lib/
make
mkdir lib
cp ./build/libcartreset7.a ./lib/libcartreset7.a
mv Makefile Makefile.arm7

mv Makefile.arm9 Makefile
make clean
make
cp ./build/libcartreset9.a ./lib/libcartreset9.a
mv Makefile Makefile.arm9

rm -rf build/
