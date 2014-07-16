AppBox
======

A runtime sandbox for Android apps based on strace. http://mxia.me/appbox

## Get started

### Dependencies

* You need standard Android development toolchain to build and upload AppBox.
* Basically you need adb from Android SDK, preferable api-15 (4.1.2).
* Also you need Android NDK. Make sure both SDK and NDK are installed properly on the machine and their binaries are in PATH.

### Build!
* Got to the root dir of the project, type `ndk-build` and that is it.
* The generated program should be under `libs/strace`. The program name remains `strace` since we try to maintain a small patch to the vanilla `strace`.

### Test!
* A real device is preferable. Emulator is not tested.
* Use adb to upload to program to the sdcard: `adb push libs/strace /sdcard`.
* Copy it over to the program folder on the device (sdcard files are not executable): `adb shell`, `su`, `cat /sdcard/strace /data/local/tmp/sandbox`, `chmod 751 /data/local/tmp/sandbox`.
* Run it in adb shell, its commandline is the same as `strace`. 
* At runtime, you will see both terminal output (vanilla `strace` outputs) and logcat output (our patch).
* GDB debugging is possible, but might be nasty cuz the sandbox itself is stracing some other process. It might worth having a try later. 
