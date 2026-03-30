[中文版](README_zh.md) | English

# Qtrace
QBDI based Android arm64 trace tool

# Features
* arm64 code Instrumentation,memory read/write monitor
* custome function monitor
* custome jni monitor
* custome libc monitor

# Usage
1.push your target so file to /data/local/tmp

2.under root ,run cmd: setenforce 0

3.modify void config() in /nativelib/cpp/native_main.cpp

4.add custome function monitor in qbdihook.cpp，add custome libc function monitor in libctrace.cpp，add custome jni function monitoe in jnitrace.cpp

5.run Build-Generate Apks,push the output lib file: libnativelib.so , to /data/local/tmp

6.inject libnativelib.so into the target process ，you can use the frida script inject.js inside this project
