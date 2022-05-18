#include "util.h"

// Send functions
int send_fyi(player *current_player)
{
    net_info *client_info = current_player->client_info;
    // Sending message
    int sent = sendto(client_info->socket, fyi, 3 * fyi[1] + 2, 0, (const struct sockaddr *)&client_info->servaddr, client_info->servaddr_size);
    if (sent == -1)
    {
        fprintf(stderr, "Error sending message (%d %s)\n", errno, strerror(errno));
        exit(1);
    }
    return sent;
}

int send_mym(player *current_player)
{
    net_info *client_info = current_player->client_info;
    char message[1];
    message[0] = MYM;
    // Sending message
    int sent = sendto(client_info->socket, message, 1, 0, (const struct sockaddr *)&client_info->servaddr, client_info->servaddr_size);
    if (sent == -1)
    {
        fprintf(stderr, "Error sending message (%d %s)\n", errno, strerror(errno));
        exit(1);
    }
    return sent;
}

int send_end(int game_state)
{
    char message[10];
    message[0] = END;
    message[1] = game_state;
    if (sendto(player_1.client_info->socket, message, sizeof(message), 0, (const struct sockaddr *)&player_1.client_info->servaddr, player_1.client_info->servaddr_size) == -1)
    {
        fprintf(stderr, "Error sending message to player 1 (%d %s)\n", errno, strerror(errno));
        exit(1);
    }
    if (sendto(player_2.client_info->socket, message, sizeof(message), 0, (const struct sockaddr *)&player_2.client_info->servaddr, player_2.client_info->servaddr_size) == -1)
    {
        fprintf(stderr, "Error sending message to player 2 (%d %s)\n", errno, strerror(errno));
        exit(1);
    }
    return 0;
}

int send_txt(char *txt, net_info *info)
{
    // Adding TXT before text
    char message[strlen(txt) + 1];
    message[0] = TXT;
    char *start = message + 1;
    strncpy(start, txt, strlen(txt));

    // Sending message
    int sent = sendto(info->socket, message, strlen(message), 0, (const struct sockaddr *)&info->servaddr, info->servaddr_size);
    if (sent == -1)
    {
        fprintf(stderr, "Error sending message (%d %s)\n", errno, strerror(errno));
        exit(1);
    }

    return sent;
}

int send_mov(int row, int column, net_info *info)
{
    // Constructing message
    char message[3];
    message[0] = MOV;
    message[1] = column;
    message[2] = row;

    // Sending message
    int sent = sendto(info->socket, message, sizeof(message), 0, (const struct sockaddr *)&info->servaddr, info->servaddr_size);
    if (sent == -1)
    {
        fprintf(stderr, "Error sending message (%d %s)\n", errno, strerror(errno));
        exit(1);
    }

    return sent;
}

void display_board(char *board)
{

    // Unpacking
    char board_seq[9];
    memcpy(board_seq, "         ", 9);
    int i = 2;
    while (i < 3 * board[1] + 2)
    {
        char player = board[i];
        char col = board[i + 1];
        char row = board[i + 2];

        if (player == CLIENT_ID)
        {
            board_seq[3 * row + col] = MY_SIGN;
        }
        else
        {
            board_seq[3 * row + col] = OTHER_SIGN;
        }
        i += 3;
    }

    // Transforming str into board to print
    for (i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            printf("%c", board_seq[3 * i + j]);
            if (j < 2)
            {
                printf("|");
            }
        }
        if (i < 2)
        {
            printf("\n-+-+-\n");
        }
    }
    printf("\n");
}

// Changes cur_player to other player by checking player.id
void switch_player(player *cur_player)
{
    if (cur_player->id == 1)
    {
        cur_player->id = 2;
        cur_player->client_info = player_2.client_info;
    }
    else if (cur_player->id == 2)
    {
        cur_player->id = 1;
        cur_player->client_info = player_1.client_info;
    }
}

// Receives mov message and saves (col, row) in the struct pointed to by requested_move
void recv_mov(net_info *client_info, move *requested_move)
{

    char response[10];
    while (response[0] != MOV) {
        int received = recvfrom(client_info->socket, (char *)response, sizeof(response), 0, (struct sockaddr *)&client_info->servaddr, (socklen_t *)&client_info->servaddr_size);
        if (received == -1)
        {
            fprintf(stderr, "Error recvfrom() (receiving move) (%d %s)\n", errno, strerror(errno));
            exit(1);
        }
        if (response[0] == TXT) {
            printf("Rejected request. Game in progress.\n");
            char message[10];
            message[0] = END;
            message[1] = 0xFF;
            if (sendto(client_info->socket, message, sizeof(message), 0, (const struct sockaddr *)&client_info->servaddr, client_info->servaddr_size) == -1)
            {
                fprintf(stderr, "Error sending message to player trying to join (%d %s)\n", errno, strerror(errno));
                exit(1);
            }
        }
    }
    requested_move->col = response[1];
    requested_move->row = response[2];
}

// Checks if move is valid
int is_valid(move *requested_move, char *board)
{
    int val = board[3 * requested_move->row + requested_move->col];
    int in_interval = ((requested_move->row >= 0) && (requested_move->row <= 2)) && ((requested_move->col >= 0) && (requested_move->col <= 2));
    if ((val == 5) && in_interval)
    {
        return 1;
    }
    return 0;
}

// Board functions
void update_board(player *cur_player, move *requested_move, char *board)
{
    board[3 * requested_move->row + requested_move->col] = cur_player->id;
    int n_moves = fyi[1];
    fyi[3 * n_moves + 2] = cur_player->id;
    fyi[3 * n_moves + 3] = requested_move->col;
    fyi[3 * n_moves + 4] = requested_move->row;
    fyi[1] = n_moves + 1;
}

// Checks if game ended. If game ended returns winner (0, 1, 2) otherwise return -1
int check_equal_nonzero_player(int pos1, int pos2, int pos3, int playerid, char *board)
{
    if ((board[pos1] == board[pos2]) && ((board[pos2] == board[pos3]) && (board[pos3] == playerid)))
    {
        return board[pos3];
    }
    else
    {
        return 0;
    }
}

int full_board(char *board)
{
    int i = 0;
    while (i < 9)
    {
        if (board[i] == 5)
        {
            return 0;
        }
        i++;
    }
    return 1;
}

int won(int playerid, char *board)
{
    int t1 = check_equal_nonzero_player(0, 1, 2, playerid, board);
    int t2 = check_equal_nonzero_player(3, 4, 5, playerid, board);
    int t3 = check_equal_nonzero_player(6, 7, 8, playerid, board);
    int t4 = check_equal_nonzero_player(0, 3, 6, playerid, board);
    int t5 = check_equal_nonzero_player(1, 4, 7, playerid, board);
    int t6 = check_equal_nonzero_player(2, 5, 8, playerid, board);
    int t7 = check_equal_nonzero_player(0, 4, 8, playerid, board);
    int t8 = check_equal_nonzero_player(2, 4, 6, playerid, board);
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 > 0;
}

int game_ended(char *board)
{
    if (won(1, board))
    {
        return 1;
    }
    if (won(2, board))
    {
        return 2;
    }
    if (full_board(board) == 1)
    {
        return 0;
    }
    return -1;
}
