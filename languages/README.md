# CAMotics Translations
CAMotics uses Qt's tools for translations.

## Editing translation files
First update the translation files with:

    lupdate qt/*.ui src/camotics/qt/*.cpp -ts languages/*.ts

Then use ``linguist`` from the Qt pacakge to edit the ``.ts`` files in
``translations``.

Finally, rebuild the software.

## Adding a new language
Replace ``<lang>`` with the two digit language code in the commands below.

Add a new 64x64 pixel flag icon to ``qt/flags/<lang>.png``.

    lupdate qt/*.ui src/camotics/qt/*.cpp -ts languages/camotics_<lang>.ts
    git add languages/camotics_<lang>.ts qt/flags/<lang>.png
