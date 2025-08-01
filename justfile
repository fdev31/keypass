build: build-index
    pio run -t compiledb
    pio run

upload: build-index
    ./scripts/update_version.sh
    pio run -t upload

clean:
    rm -fr .pio
    cd android && ./gradlew clean

keymap:
    cd scripts && ./generate keymaps/fr


apk-setup:
    @echo "Setting up Android SDK permissions and accepting licenses..."
    sudo chown -R $(whoami):$(whoami) /opt/android-sdk
    yes | sdkmanager --licenses
    @echo "Ensuring AndroidX is enabled..."
    cd android && echo "android.useAndroidX=true" >> gradle.properties
    @echo "Setup complete. You might need to run 'just clean' and 'just build' next."

# Build the debug APK
apk-build:
    cd android && ./gradlew assembleDebug

# Install the debug APK on a connected device/emulator
apk-install: apk-build
    cd android && ./gradlew installDebug

# Run the application on a connected device/emulator
# Requires 'adb' to be in your PATH and a device/emulator connected.
apk-run: apk-install
    @echo "Starting the application..."
    cd android && adb shell am start -n com.example.keypass/.MainActivity

# Run unit tests
apk-test-unit:
    cd android && ./gradlew testDebugUnitTest

# Run connected (instrumented) tests
# Requires a device/emulator connected.
apk-test-connected:
    cd android && ./gradlew connectedDebugAndroidTest

# Full build and install (default)
default: build

apk-serve:
    cd android/app/build/outputs/apk/debug && python -m http.server 9090

apk-logs:
    cd android && adb logcat -s "KeyPass:V"
    # cd android && adb logcat -v long
apk-fatal-logs:
    cd android && adb logcat -s "AndroidRuntime:E" "FATAL EXCEPTION:E"


build-index:
    #!/usr/bin/env bash
    if [[ ! -f "./src/indexPage.h" ]] || \
       [[ "portal/index.html" -nt "./src/indexPage.h" ]] || \
       [[ "portal/index.js" -nt "./src/indexPage.h" ]]; then
        echo "Dependencies are newer, rebuilding..."
        ./genIndexPage.sh
    else
        echo "Target is up to date"
    fi
