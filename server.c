#include "util.c"

int main(int argc, char *argv[])
{
    // Check for port number
    if (argc < 2)
    {
        fprintf(stderr, "Please input port number");
        exit(1);
    }

    // Store port number
    int PORT_NUMBER = atoi(argv[1]);

    // Initialise server socket
    int servsocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (servsocket == -1)
    {
        fprintf(stderr, "Error creating socket (%d %s)\n", errno, strerror(errno));
        exit(1);
    }

    // Build address and portnumber
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT_NUMBER);

    // Bind address and portnumber to server socket
    if (bind(servsocket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "Error bind() (%d %s)\n", errno, strerror(errno));
        exit(1);
    }

    n_connected_clients = 0;

    // Listen for two clients to connect
    while (n_connected_clients < 2)
    {
        // Initialise variable to store client address
        struct sockaddr_in clientaddr;
        int len = sizeof(clientaddr);

        // Initialise buffer to store received message from client
        int REC_MESS_SIZE = 1000;
        char rec_message[REC_MESS_SIZE];

        // Receive message, store it in the buffer and save client address
        int num_rec = recvfrom(servsocket, rec_message, REC_MESS_SIZE, 0, (struct sockaddr *)&clientaddr, (socklen_t *)&len);

        // Check for reception error
        if (num_rec == -1)
        {
            fprintf(stderr, "Error recvfrom() (%d %s)\n", errno, strerror(errno));
        }

        // Only connect with client when message is "4Hello\0"
        if (strcmp(my_hello, rec_message))
        {
            // If first client initialise player_1
            if (n_connected_clients == 0)
            {
                client_info_1.servaddr = clientaddr;
                client_info_1.servaddr_size = sizeof(clientaddr);
                client_info_1.socket = servsocket;
                player_1.id = 1;
                player_1.client_info = &client_info_1;
                char *greeting = "Welcome to the game! You are player number: 1, You play with sign: X\n";
                send_txt(greeting, player_1.client_info);
            }
            // If second client initialise player_2
            else
            {
                client_info_2.servaddr = clientaddr;
                client_info_2.servaddr_size = sizeof(clientaddr);
                client_info_2.socket = servsocket;
                player_2.id = 2;
                player_2.client_info = &client_info_2;
                char *greeting = "Welcome to the game! You are player number: 2, You play with token: O\n";
                send_txt(greeting, player_2.client_info);
            }
            n_connected_clients++;
        }
    }

    char *serv_board = malloc(9 * sizeof(char));
    memset(serv_board, 5, 9);

    int i = 1;
    player *cur_player;
    while (game_ended(serv_board) == -1)
    {
        if (i == 1) {
            cur_player = &player_1;
        } else {
            cur_player = &player_2;
        }
        i = (i+1)%2;

        send_fyi(cur_player);
        send_mym(cur_player);

        move requested_move;
        recv_mov(cur_player->client_info, &requested_move);

        while (is_valid(&requested_move, serv_board) == 0)
        {
            char *invalid_txt = "Your move is invalid, enter new one\n";
            send_txt(invalid_txt, cur_player->client_info);
            send_mym(cur_player);
            recv_mov(cur_player->client_info, &requested_move);
        }

        update_board(cur_player, &requested_move, serv_board);
    }

    int winner = game_ended(serv_board);
    send_end(winner);

    close(servsocket);
    free(serv_board);
    return 0;
}
