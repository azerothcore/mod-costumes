# SQL Generator

This tool generates a SQL file to insert rows into the ``item_template`` and ``costume`` tables from a single source of data.

## Requirements

* [Node.js](https://nodejs.org/en/download/)

## Usage

1. If you're using the tool for the first time, copy `costumes.template.csv` to `costumes.csv`.
2. Edit the `costumes.csv` file, add a row per costume. See the [CSV Columns](#csv-columns) section for more detail about each column.
3. Open a terminal or command prompt window, and navigate to `tools/costume-sql-generator`.
4. Before first use or after updating your local clone:
```sh
npm install
```
5. Convert the CSV file to a SQL file:
```sh
npm start
```
6. Run the SQL queries from `costumes.sql` against your world database using your favourite database client.

⚠️ Any manual edit to the `costumes.sql` file will be lost when running the tool again! Only change the data in the CSV file.

## CSV Columns

* **itemId**: The entry of the item used for this costume.
* **name**: The item's name.
* **description**: The description in the item's in-game tooltip.
* **model**: The model the player will have in-game when using this costume. This is the same value you would use with the `.morph` command. For instance, you can use the search bar on [wow.tools](https://wow.tools/dbc/?dbc=creaturedisplayinfo&build=3.3.5.12340#page=1) and use the value of the first column (`ID`).
* **scale**: The scale applied to the player when using the costume, `1.0` is the regular scale, anything between `0` and `1` will shrink the player, anything greater than `1` will grow the player.
* **duration**: The costume's duration in seconds. This overrides the default duration set in `costumes.conf`. If you want to use the module config's default duration for this costume, set the column to `-1`.
* **icon**: The display id of the item, changes its icon in-game. For instance, look for an item with an icon you like, search for the item's id on [wow.tools](https://wow.tools/dbc/?dbc=item&build=3.3.5.12340#page=1) and use the value of the `DisplayInfoID` column.
* **quality**: The item's quality. See the [AzerothCore Wiki](https://www.azerothcore.org/wiki/item_template#quality) for possible values.
* **flags**: The item's flags. See the [AzerothCore Wiki](https://www.azerothcore.org/wiki/item_template#flags) for possible values. `134217728` (Bound to Account) is an interesting flag for costumes.
* **flagsExtra**: The item's extra flags. See the [AzerothCore Wiki](https://www.azerothcore.org/wiki/item_template#flagsextra) for possible values.
* **material**: The item's material, changes the sound made when the item is moved around in the inventory. See the [AzerothCore Wiki](https://www.azerothcore.org/wiki/item_template#material) for possible values.
* **spell**: The spell triggered when right-clicking the item. This must match the `SpellId` value in the module config file.
* **cooldown**: The costume's cooldown in seconds. This overrides the default cooldown set in `costumes.conf`. If you want to use the module config's default cooldown for this costume, set the column to `-1`.
* **sound**: The ID of the sound played when using the costume. Set to `0` if you don't want a sound. You can find a list of sounds [here for example](https://wotlkdb.com/?sounds) (the ID will be displayed in the address bar when clicking on a sound).
