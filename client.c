#include "util.c"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Please input IP address and port number");
        exit(1);
    }

    int mysocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mysocket == -1)
    {
        fprintf(stderr, "Error creating socket (%d %s)\n", errno, strerror(errno));
        exit(1);
    }

    char *IP_ADDRESS = argv[1];
    short PORT_NUMBER = atoi(argv[2]);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_NUMBER);
    int serv_addr_size = sizeof(servaddr);
    inet_pton(AF_INET, IP_ADDRESS, &servaddr.sin_addr.s_addr);

    net_info info = {servaddr, serv_addr_size, mysocket};
    char * txt = "Hello\0";
    if (send_txt(txt, &info)==-1) {
        printf("Couldn't ask to join game\n");
    }

    char greeting[RESPONSE_SIZE];
    int received = recvfrom(mysocket, (char *)greeting, sizeof(greeting), 0, (struct sockaddr *)&servaddr, (socklen_t *)&serv_addr_size);
    if (received == -1)
    {
        fprintf(stderr, "Error recvfrom() (%d %s)\n", errno, strerror(errno));
        exit(1);
    }

    // Game in progress case
    if (greeting[0] == END) {
        printf("Game in progress try later\n");
        exit(1);
    }

    printf("%s\n", greeting);

    char *id_str = strchr(greeting, ':');
    if (*(id_str+2) == '1') {
        CLIENT_ID = 1;
    } else if (*(id_str+2) == '2') {
        CLIENT_ID = 2;
    }

    if (CLIENT_ID == 1) {
        MY_SIGN = 'X';
        OTHER_SIGN = 'O';
    } else if (CLIENT_ID == 2) {
        MY_SIGN = 'O';
        OTHER_SIGN = 'X';
    }

    while (1) {
        char response[RESPONSE_SIZE];

        int received = recvfrom(mysocket, (char *)response, sizeof(response), 0, (struct sockaddr *)&servaddr, (socklen_t *)&serv_addr_size);
        if (received == -1)
        {
            fprintf(stderr, "Error recvfrom() (%d %s)\n", errno, strerror(errno));
            exit(1);
        }

        response[received] = '\0';

        int code = response[0];

        if (code == FYI)
        {
            printf("Here is the current board:\n");
            display_board(response);
            printf("\n");
        } 
        else if (code == TXT)
        {
            printf("%s\n", response + 1);
        }
        else if (code == MYM)
        {
            int row, column;
            printf("Enter column & row (example: '1 2' or '2 0') \n");
            scanf("%d %d", &column, &row);
            if (send_mov(row, column, &info)==-1)
            {
                printf("Could not send move\n");
            }
            printf("\n");
        }
        else if (code == END)
        {
            if (response[1] == CLIENT_ID)
            {
                printf("You won!\n");
            }
            else if (response[1] == 0xFF)
            {
                printf("Game in progress. Try later\n");
            }
            else if (response[1] == 0) {
                printf("Draw!\n");
            }
            else if (response[1] == 3 - CLIENT_ID)
            {
                printf("You lost!\n");
            }
            return 0;
        }
        else
        {
            printf("Received wrong message\n");
        }
    }
    return 0;
}