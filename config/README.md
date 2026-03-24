# Dune Legacy Configuration Files

## Overview

This directory contains **READ-ONLY TEMPLATE FILES** for Dune Legacy configuration. These files are part of the game installation and should not be modified directly.

## User Configuration Location

The game automatically copies these template files to your user configuration directory on first run. To customize game behavior, you should edit the files in your **user directory**, not here.

### User Configuration Directories by Platform

- **Windows**: `C:\Users\<username>\AppData\Roaming\dunelegacy\config\`
- **Linux**: `~/.config/dunelegacy/config/`
- **macOS**: `~/Library/Application Support/Dune Legacy/config/`

## Configuration Files

### ObjectData.ini

Customizes unit and structure properties including:
- Hit points, price, power consumption
- Weapon damage, range, and reload time
- Movement speed, build time, tech requirements
- House-specific overrides

**Important**: For multiplayer games, all players must have identical `ObjectData.ini` values to prevent desynchronization.

### QuantBot Config.ini

Configures AI behavior including:
- Difficulty settings (Defend, Easy, Medium, Hard, Brutal)
- Attack behavior and timing
- Resource gathering limits
- Unit composition ratios per house
- Campaign vs. Custom game settings

### Dune Legacy.ini

General game configuration including:
- Video settings (resolution, fullscreen, scaling)
- Audio settings (music type, volume)
- Game options (concrete required, fog of war, etc.)
- Player name and language

## How It Works

1. **First Run**: When you launch the game for the first time (or when a config file is missing), Dune Legacy automatically copies the template from this directory to your user directory.

2. **Subsequent Runs**: The game always loads configuration from your user directory, ignoring files in this installation directory.

3. **Resetting Configuration**: To reset a configuration file to defaults, simply delete it from your user directory. The game will recreate it from the template on next launch.

## Modifying Configuration

To customize your game:

1. Navigate to your user configuration directory (see paths above)
2. Edit the desired `.ini` file with any text editor
3. Save your changes
4. Restart Dune Legacy to apply

## Multiplayer Compatibility

**Critical**: When playing multiplayer games, ensure all players have:
- **Identical `ObjectData.ini` settings** - Differences will cause desynchronization
- **Identical `QuantBot Config.ini` settings** if playing against AI - Ensures fair and consistent AI behavior

You can verify configuration consistency by comparing config hashes in the game logs.

## Troubleshooting

### "My changes aren't taking effect"

You may be editing the template files in the installation directory instead of the active files in your user directory. Always edit files in your user configuration directory (see paths above).

### "I want to reset to defaults"

Delete the config file(s) from your user directory. The game will automatically recreate them from the templates on next launch.

### "Where are my user config files?"

Run Dune Legacy and check the log file (also in your user directory). The game logs the exact paths it's using for configuration files.

## Version Information

These templates are updated with each Dune Legacy release. Your user configuration files are preserved across updates, but you may want to review the templates for new options or changes.

## More Information

For detailed documentation about configuration options and modding, visit:
- https://dunelegacy.sourceforge.net/website/development/modding.html
- https://dunelegacy.sourceforge.net/

## File Details

- `Dune Legacy.ini` - **Template file** - General game settings
- `ObjectData.ini` - **Template file** - Unit/structure properties
- `QuantBot Config.ini` - **Template file** - AI behavior configuration

**Remember**: These are templates. Edit the copies in your user directory, not these files!

