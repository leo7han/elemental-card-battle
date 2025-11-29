# Elemental Card Battle

A networked multiplayer card battle game written in C using client-server socket programming.

## Features

- 🎮 **Turn-based card battles** between two players
- 🔥 **Elemental system** with advantages/disadvantages
- 🌐 **Networked multiplayer** using TCP sockets
- 🃏 **JSON-based card data** for easy customization
- 🏆 **Win condition** - defeat all opponent's cards

## Element System

- **Fire** 🔥 > Nature 🌿
- **Nature** 🌿 > Water 💧  
- **Water** 💧 > Fire 🔥

Elemental advantage gives 2x damage, disadvantage gives 0.5x damage.

## Project Structure

elemental-card-battle/
├── server/ # Server source code
├── client/ # Client source code
├── data/ # Card data (JSON files)
├── docs/ # Documentation
└── Makefile # Build configuration


## Building and Running

### Prerequisites
- GCC compiler
- Linux/Unix environment (WSL works great)

### Compilation
# Build server
gcc server/server.c server/game.c server/cJSON.c -o battle

# Build client  
gcc client/client.c -o client_bin

# Running the Game
Start the server:

./battle
In separate terminals, run two clients:

./client_bin

# Gameplay
Two players connect to the server

Each player gets 3 random cards with elements

Players take turns attacking opponent's cards

Use elemental advantages for bonus damage

First to defeat all opponent's cards wins!

# Technical Details
Language: C11

Networking: POSIX sockets (TCP)

Protocol: Custom text-based protocol

Concurrency: Sequential turn-based (2 players)

Data Format: JSON for card configuration

# Team
CSMC312 Final Project 

