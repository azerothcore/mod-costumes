# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore
## Costumes

This is a module for [AzerothCore](http://www.azerothcore.org) that adds costumes, which temporarily morph characters.


## How to install

### 1) Simply place the module under the `modules` folder of your AzerothCore source folder.

You can clone it via git under the `azerothcore/modules` directory:

```
cd path/to/azerothcore/modules
git clone https://github.com/azerothcore/mod-costumes.git
```

or you can manually [download the module](https://github.com/azerothcore/mod-costumes/archive/refs/heads/master.zip), unzip the archive and place it under the `azerothcore/modules` directory. Make sure to rename the folder to remove the `-master` part.

### 2) Re-run CMake and build AzerothCore

### (Optional) Edit module configuration

If you need to change the module configuration, go to your server configuration folder (e.g. **etc**), copy `costumes.conf.dist` to `costumes.conf` and edit it as you prefer.

### 3) Apply the SQL queries

Run the SQL files in the `data/sql` directory.  
Alternatively, you can just start your worldserver if the updates system is enabled, it will apply the SQL queries automatically.


## How to use

1. Create a custom item by inserting a row in the `item_template` table of the world database. Using [Keira3](https://github.com/azerothcore/Keira3) makes this way easier.
2. Insert a row in the `costume` table of the world database.
    * **`item_entry`**: Must match the entry of the item added in step 1.
    * **`display_id`**: The model the player will have in-game when using this costume. This is the same value you would use with the `.morph` command. For instance, you can use the search bar on [wow.tools](https://wow.tools/dbc/?dbc=creaturedisplayinfo&build=3.3.5.12340#page=1) and use the value of the first column (`ID`).
    * **`scale`**: The scale applied to the player when using the costume, `1.0` is the regular scale, anything between `0` and `1` will shrink the player, anything greater than `1` will grow the player.
    * **`duration`**: The costume's duration in seconds. This overrides the default duration set in `costumes.conf`. If you want to use the module config's default duration for this costume, set the column to `-1`.
3. Start or restart your worldserver.
4. Log in, `.additem` your custom item with the entry used in step 1.

*Note: this process can be simplified by generating SQL queries using [tools/costume-sql-generator](tools/costume-sql-generator).*


## License

This module is released under the [GNU AGPL license](https://github.com/azerothcore/mod-costumes/blob/master/LICENSE).


## Credits

* Inspired by and adapted from [AzerothShard's ExtraItems module](https://github.com/AzerothShard/azerothshard-core/tree/development/modules/ExtraItems)
* [r-o-b-o-t-o](https://github.com/r-o-b-o-t-o): initial development
* See the [Contributors](https://github.com/azerothcore/mod-costumes/graphs/contributors) page
