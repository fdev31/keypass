keymaps := "./keymaps/fr ./keymaps/us"
# Builds the firmware
build: gen-index gen-version gen-keymap
    pio run -t compiledb
    pio run

# Upload to the MCU
upload: build
    pio run -t upload

# Remove all build files
clean:
    rm -fr .pio
    cd android && ./gradlew clean

# Attempt to setup Android license acknowledgments
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

# Run a debug session
apk-serve:
    cd android/app/build/outputs/apk/debug && python -m http.server 9090

# Show android logs
apk-logs:
    cd android && adb logcat -s "KeyPass:V"
    # cd android && adb logcat -v long

# Show android fatal logs
apk-fatal-logs:
    cd android && adb logcat -s "AndroidRuntime:E" "FATAL EXCEPTION:E"


# build the MCU indexPage.h from the .html & .css sources
gen-index:
    #!/usr/bin/env sh
    if [[ ! -f "./src/indexPage.h" ]] || \
       [[ "portal/index.html" -nt "./src/indexPage.h" ]] || \
       [[ "portal/index.js" -nt "./src/indexPage.h" ]]; then
        echo "Dependencies are newer, rebuilding..."
        ./genIndexPage.sh
    else
        echo "Target is up to date"
    fi

# update the version.h file with latest tag
gen-version:
    #!/usr/bin/env bash
    # Get the latest tag from git
    tag=$(git describe --tags --abbrev=0)
    cat <<EOF >  ./src/version.h
    #ifndef _VERSION_NUMBER
    #define _VERSION_NUMBER
    #define VERSION "$tag"
    #endif
    EOF

# re-generate the keymaps definition
gen-keymap:
    #!/usr/bin/env sh
    pushd scripts
    ./parse_keymap.py {{keymaps}} | ./gen_structures.py > ../src/keymap.h
    popd


