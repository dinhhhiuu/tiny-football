# ⚽ Tiny Football

Tiny Football is a fast-paced 2D arcade football (soccer) game featuring intense 3 vs 3 matches.
Designed for quick gameplay sessions, strategic positioning, and dynamic AI behavior, Tiny Football delivers competitive fun whether you are playing against a friend or challenging the computer.

## Game Overview

Tiny Football focuses on:
- Small teams (3 players per side)
- High-tempo gameplay
- Tactical AI positioning
- Clean and responsive mechanics
- Simple build and run workflow

Each team consists of three players dynamically assigned tactical roles during gameplay. The reduced team size increases action density and forces smarter positioning and quick decision-making.

## Game Modes
### Player vs Player (PvP)

- Two human players compete locally.
- Each player controls a team of 3.
- Perfect for competitive matches with friends.
- Fast, chaotic, and skill-based gameplay.

### Player vs NPC (PvE)

- One human player competes against AI-controlled opponents.
- AI players dynamically rotate roles based on game state.
- Designed to simulate basic football tactics in a lightweight 2D system.

## AI System

The NPC team uses a dynamic tactical role system, including:

### Chaser
- The closest player to the ball.
- Aggressively pursues the ball.
- Attempts to shoot when in range.

### Supporter
- Positions dynamically relative to the Chaser.
- Creates passing lanes.
- Mirrors movement to provide strategic spacing.

### Defender
- Stays between the ball and own goal.
- Guards defensive zones.
- Occasionally presses forward when the ball enters the danger area (1/4 of the field).

Roles rotate automatically based on ball proximity, creating fluid and dynamic behavior rather than static positioning.

## Build & Run Instructions

This project is built using MinGW (Windows environment).

### Clean the project
```bash
mingw32-make clean
```
### Build the project
```bash
mingw32-make 
```

### Run the game
```bash
mingw32-make run
```

Make sure MinGW and required dependencies are properly installed before building.

## Authors

- dinhhhiuu
- Hoai-Vu-ct
- tungngo2525