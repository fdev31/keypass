build:
    ./genIndexPage.sh
    pio run -t compiledb
    pio run

upload:
    ./genIndexPage.sh
    pio run -t upload

clean:
    rm -fr .pio

keymap:
    cd scripts && ./generate keymaps/fr
