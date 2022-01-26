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


## License

This module is released under the [GNU AGPL license](https://github.com/azerothcore/mod-costumes/blob/master/LICENSE).


## Credits

* Inspired by and adapted from [AzerothShard's ExtraItems module](https://github.com/AzerothShard/azerothshard-core/tree/development/modules/ExtraItems)
* [r-o-b-o-t-o](https://github.com/r-o-b-o-t-o): initial development
* See the [Contributors](https://github.com/azerothcore/mod-costumes/graphs/contributors) page
