[ -d android ] || mkdir android
cd android
[ -f Makefile ] || export VERBOSE="VERBOSE=1"
[ -f Makefile ] || cmake -DCMAKE_TOOLCHAIN_FILE=../android.cmake -G "Unix Makefiles" ../..
make -j2 $VERBOSE $*
