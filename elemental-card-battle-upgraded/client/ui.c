#include "ui.h"
#include <stdio.h>

// Add this function to handle battle display
void handle_battle_display(int sockfd, int player_id) {
    char buffer[1024];
    GameState game;
    
    while (1) {
        // Receive game state from server
        int bytes_received = recv(sockfd, &game, sizeof(GameState), 0);
        if (bytes_received <= 0) {
            printf("Connection lost or game ended.\n");
            break;
        }
        
        // Clear screen (optional - you can remove if it causes issues)
        // system("clear");
        
        // Display battle state
        display_battle_state(&game, player_id);
        
        // Check if game is over
        if (game.game_over) {
            display_game_over(game.winner, 
                            game.winner == player_id ? "YOU" : "Opponent");
            break;
        }
        
        // If it's this player's turn, get input
        if (game.current_player == player_id) {
            printf("\nEnter your move (attacker_index defender_index): ");
            int attacker, defender;
            
            if (scanf("%d %d", &attacker, &defender) == 2) {
                // Send move to server
                send(sockfd, &attacker, sizeof(int), 0);
                send(sockfd, &defender, sizeof(int), 0);
                
                // Wait for attack result
                char result_msg[256];
                recv(sockfd, result_msg, sizeof(result_msg), 0);
                printf("%s\n", result_msg);
            } else {
                printf("Invalid input! Please enter two numbers.\n");
                // Clear input buffer
                while (getchar() != '\n');
            }
        } else {
            printf("Waiting for opponent's move...\n");
            // Wait for opponent's move result
            char result_msg[256];
            recv(sockfd, result_msg, sizeof(result_msg), 0);
            printf("%s\n", result_msg);
        }
    }
}

void display_battle_state(GameState *game, int player_id) {
    printf("\n=== ELEMENTAL CARD BATTLE ===\n");
    
    // Show opponent's cards (face down if not revealed)
    printf("\n--- OPPONENT'S CARDS ---\n");
    int opponent_id = (player_id == 0) ? 1 : 0;
    for (int i = 0; i < 3; i++) {
        if (game->players[opponent_id].cards[i].hp > 0) {
            printf("%d: [???](HP:??) ", i);
        } else {
            printf("%d: [DEFEATED] ", i);
        }
    }
    printf("\n");
    
    // Show player's own cards with full info
    printf("\n--- YOUR CARDS ---\n");
    for (int i = 0; i < 3; i++) {
        Card *card = &game->players[player_id].cards[i];
        if (card->hp > 0) {
            printf("%d: %s(%s) HP:%d ATK:%d ", 
                   i, card->name, card->element, card->hp, card->attack);
        } else {
            printf("%d: [DEFEATED] ", i);
        }
    }
    printf("\n");
    
    // Show turn information
    if (game->current_player == player_id) {
        printf("\n🎯 YOUR TURN!\n");
    } else {
        printf("\n⏳ Waiting for opponent's move...\n");
    }
}

void display_attack_result(const char *attacker, const char *defender, 
                          int damage, const char *element_effect) {
    printf("\n⚔️  BATTLE ACTION: %s attacks %s!\n", attacker, defender);
    printf("💥 Damage: %d\n", damage);
    if (element_effect[0] != '\0') {
        printf("✨ Element Effect: %s\n", element_effect);
    }
    printf("------------------------\n");
}

void display_game_over(int winner, const char *winner_name) {
    printf("\n🎊 GAME OVER! 🎊\n");
    if (winner != -1) {
        printf("🏆 Winner: %s\n", winner_name);
    } else {
        printf("🤝 It's a draw!\n");
    }
    printf("====================\n");
}