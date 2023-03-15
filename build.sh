git clone https://github.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 gcc

wget https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86/+archive/android-9.0.0_r6/clang-4639204.tar.gz && export HOME_RN=/workspace/BlueSeax-Garden && mkdir clang && mv clang-4639204.tar.gz clang && cd clang && tar -xzvf clang-4639204.tar.gz && cd $HOME_RN

sudo apt update | sudo apt upgrade

sudo apt install binutils-aarch64-linux-gnu libncurses5 -y

clear

make -j8 O=out ARCH=arm64 CC=$(pwd)/toolchains/clang/bin/clang CROSS_COMPILE=$(pwd)/toolchains/gcc/bin/aarch64-linux-android- CLANG_TRIPLE=aarch64-linux-gnu- angelica_defconfig 

make -j8 O=out ARCH=arm64 CC=$(pwd)/toolchains/clang/bin/clang CROSS_COMPILE=$(pwd)/toolchains/gcc/bin/aarch64-linux-android- CLANG_TRIPLE=aarch64-linux-gnu-

