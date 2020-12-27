adb forward tcp:5039 tcp:5039
# adb push $ANDROID_NDK/prebuilt/android-arm64/gdbserver/gdbserver /data/local/tmp/
adb shell /data/local/tmp/gdbserver :5039 /data/local/tmp/home/chenxx/vector_crash_android/vector_crash_android --debug
