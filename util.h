#ifndef UTIL_H
#define UTIL_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int RESPONSE_SIZE = 1000;

char FYI = 1;
char MYM = 2;
char END = 3;
char TXT = 4;
char MOV = 5;
char LFT = 6;

// Structs
typedef struct move
{
    int col;
    int row;
} move;

typedef struct net_info
{
    struct sockaddr_in servaddr;
    int servaddr_size;
    int socket;
} net_info;

typedef struct player
{
    int id;
    net_info *client_info;
} player;

char CLIENT_ID = 1;
char MY_SIGN;
char OTHER_SIGN;

int n_connected_clients = 0;
player player_1, player_2;
char *my_hello = "\4hello\0";
char fyi[100] = "\1\0";

net_info client_info_1;
net_info client_info_2;



int send_fyi(player *current_player);

int send_mym(player *current_player);

int send_end(int game_state);

int send_txt(char *txt, net_info *info);

int send_mov(int row, int column, net_info *info);

void display_board(char *board);

void switch_player(player *cur_player);

void recv_mov(net_info *client_info, move *requested_move);

int is_valid(move *requested_move, char *board);

void update_board(player *cur_player, move *requested_move, char *board);

int check_equal_nonzero_player(int pos1, int pos2, int pos3, int playerid, char *board);

int full_board(char *board);

int won(int playerid, char *board);

int game_ended(char *board);

#endif