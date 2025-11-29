#ifndef GAME_H
#define GAME_H

typedef struct {
    char name[50];
    char element[20];
    int attack;
    int hp;           // Use this as the main health field
    int health;       // Keep for compatibility
} Card;

// Function declarations
int elemental_advantage(char *attacker, char *defender);
int attack(Card *attacker, Card *defender);

#endif