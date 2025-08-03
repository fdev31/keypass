keymaps := "./keymaps/fr ./keymaps/us"
# Builds the firmware
build: gen-index gen-version gen-keymap gen-icons
    pio run -t compiledb
    pio run

# Upload to the MCU
upload: build
    pio run -t upload

# Remove all build files
clean:
    rm -fr .pio
    cd android && ./gradlew clean

# Full build and install (default)
default: build

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
    pushd src
    ../scripts/genkeymaps.py -o keymap ../scripts/keymaps/{fr,us}
    popd

gen-icons:
    #!/usr/bin/env sh
    pushd src
    ../scripts/png2icon.py -o icons ../images/icon_{bluetooth,wifi,up,down}.png
    popd
