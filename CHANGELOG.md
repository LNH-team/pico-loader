# Changelog

## [Unreleased]

### Added
- DMA card read support for the DSTT platform - by @lifehackerhansol
- DMA card read support for the M3DS platform - by @lifehackerhansol
- AP patches for NRL Mascot Mania and AFL Mascot Manor - by @taxicat1
- Houkago Shounen race condition patch - by @taxicat1

### Fixed
- DMA card read offsets for Pokemon Ranger (EU) - by @taxicat1
- Split DSTT SD read to multiple patches to avoid running out of space - by @lifehackerhansol

## [v1.6.0] - 29 Mar 2026

### Added
- Support for the SuperChis slot 2 card - by @edo9300
- Most of the remaining SDK 2-4 DMA patches - by @taxicat1
- Support for cheats
    - The Pico Loader API was updated to v3 for this purpose.

### Fixed
- Star Wars Jedi Alliance patch - by @taxicat1

## [v1.5.0] - 1 Feb 2026 - Puppy Palace Release 🐶
> [!IMPORTANT]
> With this release a new file `patchlist.bin` is introduced that needs to be copied to the `/_pico` folder on your SD card.

### Added
- AP patch for Last Window: The Secret of Cape West - by @taxicat1
- Missing CARDi_ReadRomIdCore signature for Top Spin 2 (USA) - by @taxicat1
    - Fixes Top Spin 2 (USA) on DSTT and slot 2 cards
- Patch list with misc arm9 patches and metafortress addresses - Implementation by @Gericom, patch list contents by @taxicat1
    - Puppy Palace (USA), My Puppy Shop (Europe), Machi no Pet-ya-san DS - 200 Piki Wan-chan Daishuugou (Japan)
    - Castlevania - Portrait of Ruin (USA) and (Europe), Akumajou Dracula - Gallery of Labyrinth (Japan), Akumajou Dracula - Gallery of Labyrinth (Japan) (Rev 1)
    - Luminous Arc 2 (USA) and (Europe), Luminous Arc 2 - Will (Japan)
    - Children of Mana (USA) and (Europe), Seiken Densetsu DS - Children of Mana (Japan)
    - Dementium II (USA) and (Europe), Tozasareta Byoutou - Dementium II (Japan)
    - Grand Theft Auto - Chinatown Wars (USA), (Europe) and (Japan)
    - Star Wars - The Clone Wars - Jedi Alliance (USA) and (Europe)
    - C.O.P. - The Recruit (USA) and (Europe)
    - Battle of Giants - Dragons (USA) (Rev 1), Combat of Giants - Dragons (Europe)
    - Battle of Giants - Mutant Insects (USA), Combat of Giants - Mutant Insects (Europe)
    - Captain America - Super Soldier (USA) and (Europe)
    - Imagine - Resort Owner (USA), Imagine - Dream Resort (Europe)
    - Might & Magic - Clash of Heroes (USA) and (Europe)
    - Prince of Persia - The Forgotten Sands (USA) and (Europe)
    - Kirby - Mass Attack (USA) and (Europe), Atsumete! Kirby (Japan), Moyeora! Kirby (Korea)
- Keep a valid stack pointer when booting the arm9
    - Fixes Battle of Giants - Mutant Insects (USA), Combat of Giants - Mutant Insects (Europe) together with the patch in the patch list
- System for adjusting addresses for autoload - by @taxicat1
    - Fixes New Super Mario Bros (USA)
- Attempt to fix rom hacks that decompress the arm9, but don't set compressedEnd to zero
    - Fixes Pokemon SoulSilver Deluxe (together with the fix for roms repacked in unusual ways)
- Support for CARDi_ReadCard signature used by Chou Soujuu Mecha MG - by @taxicat1

### Changed
- Reduce size of some patch codes - by @edo9300 and @Gericom
    - Fixes Dragon Quest Heroes - Rocket Slime (USA), Imagine - Fashion Designer (Europe) and Mugen Kouro (Japan)
- Improve handling of module params and locating of unused libsyscall space to support rom hacks that repack roms in unusual ways
    - Fixes Coropata (English translation)

### Fixed
- Ignore rom reads into null pointer destinations - by @taxicat1
    - Fixes Tony Hawk's Downhill Jam

## [v1.4.0] - 11 Jan 2026

### Added
- Overlay patch to fix race condition in Kirby Super Star Ultra - by @taxicat1
- Overlay patch for hombrew AP in Rabbids Go Home - by @taxicat1
- Bypass region locking on iQue games - by @R-YaTian
- Properly set MBK9 (twl wram locking) settings - by @R-YaTian
- Support for SDK variant with a lot of inlining
    - Fixes Tropix! ...Your Island Getaway and Think - Training für den Kopf - Kids - Spiel Dich Schlau!
    - Note that these two games don't work yet on platforms that need card id patches (DSTT and slot2 platforms)
- Support for Slot 2 flashcarts using Compact Flash - by @edo9300
    - Supercard CF (SUPERCARDCF)
    - GBA Media Player CF (MPCF)
    - M3 Adapter CF (M3CF)
    - Max Media Dock CF (MMCF)
- Proper detection of DS mode DSiWare
    - Fixes DS Download Play and Pictochat DSiWare apps
- Support for the homebrew bootstub that allows to return to the launcher application
    - The Pico Loader API was updated to v2 for this purpose. The new `launcherPath` field needs to be set to the path of the launcher rom. The bootstub is only inserted when `launcherPath` is specified. As such, you will also need to update your launcher. If you are using Pico Launcher, support was added in v1.1.0.

### Changed
- Duplicate entries removed from save list and missing title added - by @synthic
    - Fixes Disney Two Pack - Frozen Olaf's Quest + Big Hero 6 - Battle in the Bay (USA)
- Improve nocash print support detection to also detect melon ds - by @synthic
- Lock scfg9 when switching from DSi to DS mode
    - Fixes homebrew with a DS mode header that tries to detect DSi mode. This for example fixes touch in DSCraft.

### Fixed
- DATEL: Make read sector function vram safe - by @edo9300
    - Fixes some games that read directly to vram and fixes games that use OS_ResetSystem on the DATEL platform
- Proper handling of blx branches in CardiTryReadCardDmaPatch - by @taxicat1
    - Fixes Ninja Gaiden - Dragon Sword (USA)
- Fix sCARDiTryReadCardDmaPatternSdk3017530 matching multiple variants - by @taxicat1
    - Fixes Unou no Tatsujin - Soukai! Machigai Museum 2 (Japan), Trioncube (Europe) (En,Fr,De,Es,It), Tetris DS (Korea), Original Frisbee Disc Sports - Ultimate & Golf (USA) and LifeSigns - Surgical Unit (USA)
- Fix Shaberu! DS Oryouri Navi (Japan) (rev 0 and 1) by accounting for a spurious CARDi_ReadCard match

## [v1.3.1] - 29 Dec 2025

### Fixed
- DATEL: Put back the full spi cycle routine when sending commands - by @edo9300
    - Fixes issues with some SD cards on the DATEL platform

## [v1.3.0] - 28 Dec 2025

### Added
- Properly set the supported language, user language, and region based on the ROM's region - by @R-YaTian
    - This bypasses the region lock or language lock in some dsiware games.
- Missing OS_ResetSystem signature - by @taxicat1
    - Fixes crashes in Tetris DS and other games that have this signature.
- Support for roms with a writable NAND chip.
    - Fixes WarioWare D.I.Y., Face Training, Jam with the Band and Nintendo DS Guide.
- Special save initialization for Jam with the Band and Daigassou! Band-Brothers DX
- New supported platform: Datel (Games n' Music and Action Replay DS(i)) - by @edo9300
- New supported platform: Stargate 3DS - by @lifehackerhansol

### Changed
- Optimized space usage of supercard platform - by @edo9300
- Check in CardiTaskThreadPatch that the required slot is actually mapped to arm7.
    - Fixes Power Rangers - Samurai.

### Fixed
- Fix CardiTaskThreadPatch to apply the right patch for all signatures
    - This fixes various games, including Chibi Robo! Park Patrol, Rainbow Islands Revolution, MechAssault - Phantom War and Diddy Kong Racing DS (USA).

## [v1.2.0] - 22 Dec 2025

### Added
- Support for DSProtect patches in static arm9 - by @taxicat1
- Support for unusual DSProtect configurations - by @taxicat1
    - Fixes Dragon Ball: Origins 2 and Puyo Puyo 7
- Proper implementation of save verification
    - Fixes Madagascar, Bleach: Dark Souls and Final Fantasy Crystal Chronicles: Echoes of Time

### Changed
- Improve Pokémon IR sensor AP patches - by @taxicat1
    - Fixes Pokémon HeartGold and SoulSilver
- Always boot roms with slot 1 and 2 mapped to arm7
    - Fixes FastVideoDSPlayer homebrew
- Improve Sdk5DsiSdCardRedirectPatch

### Fixed
- Fix some retail roms being detected as homebrew
    - Fixes Scooby Doo! - Unmasked and Kim Possible - Kimmunicator
- Fix some system tool DSi roms being misdetected as DSiWare

## [v1.1.0] - 4 Dec 2025

### Added
- Support for EZ-Flash Parallel platform - by @lifehackerhansol
- Add a few more games to the DLDI patch blacklist
    - Fixes Final Fantasy Crystal Chronicles - Ring of Fates (EU and US) and Nanashi no Geemu
- Clear all arm9 CPU registers before booting a game
    - Fixes the Cake Ninja DSiWare titles

## Changed
- Simplified and improved DSProtect patches - by @taxicat1
    - Fixes Saga 3: Shadow or Light and Solatorobo: Red the Hunter

## [v1.0.1] - 29 Nov 2025

### Added
- Add Mario vs. Donkey Kong - Mini-Land Mayhem! US rev 2 (V2GE) to the AP list - by @IceKareemCheese

### Fixed
- Fix the DSProtect v2.03 patch (thanks @taxicat1)
    - Fixes Professor Layton and the Last Specter among others

## [v1.0.0] - 25 Nov 2025
- Initial release
