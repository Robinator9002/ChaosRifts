# Chaos Rift: Daemon's Ascent
[![Project Status: Active Development](https://img.shields.io/badge/status-active%20development-green.svg)](https://shields.io/)

## About the Game

**Chaos Rift: Daemon's Ascent** is an ambitious third-person action roguelite developed with Unreal Engine 5. Step into the shoes of a Chaos Daemon and fight your way through a hostile, procedurally generated world that reveals its secrets only through careful exploration. The game emphasizes responsive movement and dynamic combat, driven by a unique resource and ability system.

## Gameplay Features (Planned)

* **Fluid Movement:** Experience rapid traversal with an inherently fast character, able to strategically reposition using a tactical **dash**. Customizable dash variations (teleport, extended range, air-dash) offer strategic depth.
* **Dynamic Combat:** Combine precise **sword combat** with destructive **magic** and a powerful **Chaos Dwarf Musket**. Each weapon offers a unique playstyle.
    * **Magic:** Wielded "from the other hand" with variable cast times for tactical depth.
    * **Weapon Specialization:** A single sword and a single musket, customizable through unlockable upgrades and cosmetic skins.
* **Resource Management:** An innovative **Chaos system** serves as the primary magic resource. Gaining Chaos not only fuels your spells but also automatically recharges your sole **health potion**. A full Chaos bar enables the use of mighty **Ultimate Abilities**.
* **Ability Progression:** Unlock new skills and enhance your existing spells and dash customizations as you progress through the game.
* **Roguelite Structure:**
    * **Procedural Levels:** Each run offers new, procedurally generated environments with varying layouts, enemy spawns, and challenges. Players can even customize parameters like enemy density or level size.
    * **Long-term Motivation:** A planned **Rune System** will introduce modifiers that drastically change gameplay from run to run (e.g., enemies with enchanted weapons for increased Chaos generation). Additional unlockable level layouts, boss rush levels, and challenges ensure ongoing replayability.
    * **Environmental Storytelling:** The daemon's story and the world's lore are revealed organically through the game environment, without intrusive NPCs or extensive cinematics.

## Technical Details

* **Engine:** Unreal Engine 5.6 (C++ & Blueprints Hybrid Development)
* **Platform:** Linux (primary), other platforms potentially in the future
* **Development Environment:** Visual Studio Code

## Getting Started with Development (for Contributors / Self-Reference)

1.  **Clone the project:**
    `git clone https://github.com/Robinator9002/ChaosRifts`
2.  **Unreal Engine Setup (Linux):**
    * Ensure Unreal Engine 5.6 (preferably the `unreal-engine-bin` AUR package for Arch Linux) is installed.
    * **Verify/Set Permissions:** `sudo chown -R <your_username>:<your_username> /opt/unreal-engine` (or wherever the engine is installed)
    * **If the project is on a mounted drive:** `sudo chown -R <your_username>:<your_username> /mnt/your_mountpoint`
3.  **VS Code Setup:**
    * Install the following VS Code Extensions: `C/C++ (Microsoft)`, `Unreal Engine (Microsoft)`, `clangd`.
    * In the Unreal Editor, go to `Edit -> Editor Preferences -> General -> Source Code` and select "Visual Studio Code" as the Source Code Editor.
    * In the Unreal Editor: Run `Tools -> Refresh Visual Studio Code Project`.
4.  **Compile & Run:**
    * Open the project folder in VS Code.
    * Compile the C++ code within VS Code (`Ctrl+Shift+P` -> `Tasks: Run Build Task`).
    * Launch the Unreal Editor from the terminal: `/opt/unreal-engine/Engine/Binaries/Linux/UnrealEditor /mnt/unreal/ChaosRifts/ChaosRifts.uproject` (adjust path as needed).
