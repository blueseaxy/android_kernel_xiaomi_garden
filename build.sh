sudo apt update | sudo apt upgrade

sudo apt install binutils-aarch64-linux-gnu libncurses5 -y

clear

make -j8 O=out ARCH=arm64 CC=$(pwd)/toolchains/clang/bin/clang CROSS_COMPILE=$(pwd)/toolchains/gcc/bin/aarch64-linux-android- CLANG_TRIPLE=aarch64-linux-gnu- angelica_defconfig 

make -j8 O=out ARCH=arm64 CC=$(pwd)/toolchains/clang/bin/clang CROSS_COMPILE=$(pwd)/toolchains/gcc/bin/aarch64-linux-android- CLANG_TRIPLE=aarch64-linux-gnu-

