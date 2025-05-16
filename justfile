upload:
    pio run -t upload

clean:
    rm -fr .pio

keymap:
    cd scripts && ./generate keymaps/fr
