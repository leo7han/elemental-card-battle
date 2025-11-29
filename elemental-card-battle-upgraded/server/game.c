#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

// Returns multiplier for elemental advantage
int elemental_advantage(char *attacker, char *defender) {
    if(strcmp(attacker, "fire")==0 && strcmp(defender, "grass")==0) return 2;
    if(strcmp(attacker, "water")==0 && strcmp(defender, "fire")==0) return 2;
    if(strcmp(attacker, "grass")==0 && strcmp(defender, "water")==0) return 2;
    return 1; // normal damage
}

// Attack function applies elemental advantage
int attack(Card *attacker, Card *defender) {
    int multiplier = elemental_advantage(attacker->element, defender->element);
    int damage = attacker->attack * multiplier;
    defender->health -= damage;
    if(defender->health < 0) defender->health = 0;
    printf("%s attacks %s for %d damage!\n", attacker->name, defender->name, damage);
    return defender->health;
}

