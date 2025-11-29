#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 4096

// Function to clear the input buffer
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("🎮 Connected to Elemental Card Battle Server! 🎮\n\n");

    // Display game instructions
    printf("=== GAME RULES ===\n");
    printf("• Elements: Fire 🔥 > Nature 🌿 > Water 💧 > Fire 🔥\n");
    printf("• Element Advantage: 2x damage\n");
    printf("• Element Disadvantage: 0.5x damage\n");
    printf("• Defeat all opponent's cards to win!\n");
    printf("==================\n\n");

    while (1) {
        // Clear buffer
        memset(buffer, 0, sizeof(buffer));
        
        // Receive data from server
        int n = read(sockfd, buffer, sizeof(buffer)-1);
        if (n <= 0) {
            printf("Connection lost or game ended.\n");
            break;
        }
        buffer[n] = '\0';

        // Print server message
        printf("%s", buffer);

        // Check if it's our turn to input
        if (strstr(buffer, "Enter attacker index and defender index") != NULL) {
            int attacker, defender;
            int valid_input = 0;
            
            while (!valid_input) {
                printf("Your move: ");
                
                if (scanf("%d %d", &attacker, &defender) == 2) {
                    if (attacker >= 0 && defender >= 0 && attacker <= 2 && defender <= 2) {
                        valid_input = 1;
                    } else {
                        printf("❌ Invalid indices! Please enter numbers between 0-2 (e.g., 1 2): ");
                    }
                } else {
                    printf("❌ Invalid input! Please enter two numbers (e.g., 1 2): ");
                    clear_input_buffer();
                }
            }
            
            // Clear the input buffer after successful read
            clear_input_buffer();
            
            // Send the move to server
            snprintf(buffer, sizeof(buffer), "%d %d", attacker, defender);
            send(sockfd, buffer, strlen(buffer), 0);
            
            printf("⏳ Processing your move...\n\n");
        }
        
        // Small delay to make output more readable
        usleep(100000); // 0.1 second
    }

    printf("\nThanks for playing! 👋\n");
    close(sockfd);
    return 0;
}