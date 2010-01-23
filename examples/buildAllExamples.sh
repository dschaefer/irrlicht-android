#! /bin/bash
[ -z $1 ] || TARGET=$1
[ -z $TARGET ] && TARGET=all
for i in [012]* Demo; do
  echo "Building $i";
  pushd $i && make clean $TARGET;
  popd;
done
