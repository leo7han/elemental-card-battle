#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "game.h"
#include "cJSON.h"

#define MAX_CARDS 10
#define PORT 12345
#define MAX_CLIENTS 2

// Load cards from JSON
int load_cards(const char *filename, Card deck[], int max_cards) {
    FILE *f = fopen(filename, "r");
    if(!f) {
        printf("Failed to open %s\n", filename);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = (char*)malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON *json = cJSON_Parse(data);
    free(data);
    if(!json) {
        printf("Error parsing JSON\n");
        return 0;
    }

    int count = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {
        if(count >= max_cards) break;

        cJSON *name = cJSON_GetObjectItem(item, "name");
        cJSON *element = cJSON_GetObjectItem(item, "element");
        cJSON *attack = cJSON_GetObjectItem(item, "attack");
        cJSON *health = cJSON_GetObjectItem(item, "health");

        if(name && element && attack && health) {
            strncpy(deck[count].name, name->valuestring, sizeof(deck[count].name)-1);
            deck[count].name[sizeof(deck[count].name)-1] = '\0';
            strncpy(deck[count].element, element->valuestring, sizeof(deck[count].element)-1);
            deck[count].element[sizeof(deck[count].element)-1] = '\0';
            deck[count].attack = attack->valueint;
            deck[count].hp = health->valueint;
            deck[count].health = health->valueint; // Keep both for compatibility
            count++;
        }
    }

    cJSON_Delete(json);
    return count;
}

// Send battle state to client
void send_battle_state(int sockfd, Card player_deck[], Card opponent_deck[], int count, int is_player_turn, int player_id) {
    char buffer[2048];
    buffer[0] = '\0';
    
    // Header
    strcat(buffer, "=== ELEMENTAL CARD BATTLE ===\n\n");
    
    // Opponent's cards (hidden info)
    strcat(buffer, "--- OPPONENT'S CARDS ---\n");
    for(int i = 0; i < count; i++) {
        if(opponent_deck[i].hp > 0) {
            char tmp[128];
            sprintf(tmp, "%d: [HIDDEN](HP:UNK) ", i);  // Changed ?? to UNK
            strcat(buffer, tmp);
        } else {
            strcat(buffer, "[DEFEATED] ");
        }
    }
    strcat(buffer, "\n\n");
    
    // Player's own cards (full info)
    strcat(buffer, "--- YOUR CARDS ---\n");
    for(int i = 0; i < count; i++) {
        if(player_deck[i].hp > 0) {
            char tmp[256];
            sprintf(tmp, "%d: %s(%s) HP:%d ATK:%d ", 
                   i, player_deck[i].name, player_deck[i].element, 
                   player_deck[i].hp, player_deck[i].attack);
            strcat(buffer, tmp);
        } else {
            strcat(buffer, "[DEFEATED] ");
        }
    }
    strcat(buffer, "\n\n");
    
    // Turn information
    if(is_player_turn) {
        strcat(buffer, "YOUR TURN!\n");
    } else {
        strcat(buffer, "Waiting for opponent's move...\n");
    }
    
    send(sockfd, buffer, strlen(buffer), 0);
}

// Check if a player still has alive cards
int is_alive(Card deck[], int count){
    for(int i = 0; i < count; i++)
        if(deck[i].hp > 0) return 1;
    return 0;
}

// Send attack result to both players
void send_attack_result(int clients[2], Card *attacker, Card *defender, int damage, const char *effect) {
    char buffer[512];
    if(effect[0] != '\0') {
        snprintf(buffer, sizeof(buffer), 
                 "%s attacks %s for %d damage! %s\n",
                 attacker->name, defender->name, damage, effect);
    } else {
        snprintf(buffer, sizeof(buffer),
                 "%s attacks %s for %d damage!\n",
                 attacker->name, defender->name, damage);
    }
    
    for(int i = 0; i < 2; i++) {
        send(clients[i], buffer, strlen(buffer), 0);
    }
}

int main() {
    srand(time(NULL));

    int server_fd, clients[MAX_CLIENTS];
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0){ perror("socket failed"); exit(EXIT_FAILURE); }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if(listen(server_fd, MAX_CLIENTS) < 0){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server started. Waiting for %d players to connect...\n", MAX_CLIENTS);

    // Accept clients
    for(int i = 0; i < MAX_CLIENTS; i++){
        clients[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if(clients[i] < 0){ perror("accept"); exit(EXIT_FAILURE); }
        printf("Player %d connected.\n", i+1);
        send(clients[i], "Connected to server.\n", 21, 0);
    }

    // Load decks
    Card decks[2][MAX_CARDS];
    int counts[2];
    counts[0] = load_cards("data/player1_cards.json", decks[0], MAX_CARDS);
    counts[1] = load_cards("data/player2_cards.json", decks[1], MAX_CARDS);

    printf("Both players connected. Battle starts!\n");

    int turn = 0; // Player 1 starts

    while(is_alive(decks[0], counts[0]) && is_alive(decks[1], counts[1])) {
        int player = turn % 2;
        int opponent = 1 - player;
        char buffer[128];

        // Send battle state to both players
        for(int i = 0; i < 2; i++) {
            int is_turn = (i == player);
            send_battle_state(clients[i], decks[i], decks[1-i], counts[i], is_turn, i);
        }

        // Ask current player for attack
        send(clients[player], "Enter attacker index and defender index (e.g., 1 2): ", 55, 0);

        int a_idx = -1, d_idx = -1;
        int bytes = recv(clients[player], buffer, sizeof(buffer)-1, 0);
        if(bytes <= 0) break;
        buffer[bytes] = '\0';
        if(sscanf(buffer, "%d %d", &a_idx, &d_idx) != 2){
            send(clients[player], "Invalid input! Try again.\n", 27, 0);
            continue;
        }

        // Validate indexes
        if(a_idx < 0 || a_idx >= counts[player] || d_idx < 0 || d_idx >= counts[opponent] ||
           decks[player][a_idx].hp <= 0 || decks[opponent][d_idx].hp <= 0) {
            send(clients[player], "Invalid attack! Try again.\n", 29, 0);
            continue;
        }

        // Perform attack and get elemental effect
        int damage = attack(&decks[player][a_idx], &decks[opponent][d_idx]);
        char effect[64] = "";
        int multiplier = elemental_advantage(decks[player][a_idx].element, decks[opponent][d_idx].element);
        if(multiplier == 2) {
            strcpy(effect, "Element Advantage! 2x Damage!");
        } else if(multiplier == 0) {
            strcpy(effect, "Element Disadvantage! 0.5x Damage!");
        }

        // Notify both players
        send_attack_result(clients, &decks[player][a_idx], &decks[opponent][d_idx], damage, effect);

        // Check if defender was defeated
        if(decks[opponent][d_idx].hp <= 0) {
            char defeat_msg[128];
            snprintf(defeat_msg, sizeof(defeat_msg), "%s was defeated!\n", decks[opponent][d_idx].name);
            for(int i = 0; i < 2; i++) {
                send(clients[i], defeat_msg, strlen(defeat_msg), 0);
            }
        }

        turn++;
        
        // Small delay between turns
        sleep(1);
    }

    // Send result
    char result[128];
    if(is_alive(decks[0], counts[0])) {
        sprintf(result, "GAME OVER! Player 1 wins!\n");
    } else if(is_alive(decks[1], counts[1])) {
        sprintf(result, "GAME OVER! Player 2 wins!\n");
    } else {
        sprintf(result, "GAME OVER! It's a draw!\n");
    }

    for(int i = 0; i < 2; i++) {
        send(clients[i], result, strlen(result), 0);
    }

    printf("%s", result);

    // Close clients
    for(int i = 0; i < 2; i++) close(clients[i]);
    close(server_fd);

    return 0;
}