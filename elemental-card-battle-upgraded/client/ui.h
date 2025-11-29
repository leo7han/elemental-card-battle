#ifndef UI_H
#define UI_H

#include "game.h"

void display_battle_state(GameState *game, int player_id);
void display_attack_result(const char *attacker, const char *defender, 
                          int damage, const char *element_effect);
void display_game_over(int winner, const char *winner_name);
void display_welcome_message();
void display_help();

#endif