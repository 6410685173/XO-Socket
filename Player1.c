#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

char board[3][3] = {{' ',' ',' '},
                    {' ',' ',' '},
                    {' ',' ',' '}}; //initial board

char my_role;
char friend_role;

void socket_server(int port);
void screen();
void send_data(int soc, char *buff);
void recv_data(int soc, char *buff);
void update_board(int index,char c);
void check_winner();

int main(int argc ,char *argv[]){
    char mode[] = "";

    printf("Enter an mode(server or client): ");
    scanf("%s",mode);

    if ( strcmp(mode,"server") == 0){
        printf("server\n");
        screen();
        socket_server(5000);

    }else{
        printf("client");
    }
    return 0;
}


// create socket
void socket_server(int port){

    // config
    int listen_server = 0,conn_cli = 0;
    struct sockaddr_in servAddr;
    char buffer[1024];
    memset(&servAddr, '0', sizeof(servAddr));
    memset(buffer, '0', sizeof(buffer)); 



    listen_server = socket(AF_INET, SOCK_STREAM, 0); // use TCP connection 

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port); 

    bind(listen_server, (struct sockaddr*)&servAddr, sizeof(servAddr)); 

    listen(listen_server, 1); 

    conn_cli = accept(listen_server, (struct sockaddr*)NULL, NULL); 
    my_role = 'X';
    friend_role = 'O';
    while(1){

        send_data(conn_cli, buffer);
        screen();
        check_winner();

        recv_data(conn_cli, buffer);
        screen();
        check_winner();
    }
    
}

void send_data(int soc, char *buff){
    char position[1024] ;

    printf("Enter an your position: ");
    scanf("%s",position);

    snprintf(buff, sizeof(buff),"%s\n",position);
    write(soc, buff, strlen(buff)); 
    update_board(atoi(position),'X');
}

void recv_data(int soc, char *buff){
    char position[1024] ;

    recv(soc, buff, sizeof(buff), 0); 
    strcpy(position, buff);
    update_board(atoi(position),'O');
}

void update_board(int index,char c){

    if (0 < index < 4){
        if(board[0][index-1] == my_role || board[0][index-1] == friend_role ){
            printf("duplicate\n");
        }else{
            board[0][index-1] = c;
        }
    }else if (4 <= index < 7){       
        if(board[1][index-4] == my_role || board[1][index-4] == friend_role ){
            printf("duplicate\n");
        }else{
            board[1][index-4] = c;
        }
    }else if (7 <= index < 10){
        if(board[2][index-7] == my_role || board[2][index-7] == friend_role ){
            printf("duplicate\n");
        }else{
            board[2][index-7] = c;
        }
    }else{
        printf("wrong place\n");
    }
}
void check_winner(){
    //while(1){
        // check row
        for(int i = 0;i<3;i++){
            if(board[i][0] == my_role && board[i][1] == my_role && board[i][2] == my_role ){
                printf("The winner is you\n");
            }else if(board[i][0] == friend_role && board[i][1] == friend_role && board[i][2] == friend_role){
                printf("The winner is Player2\n");
            }
        }
        
        //check column
        for(int i = 0;i<3;i++){
            if(board[0][i] == my_role && board[1][i] == my_role && board[2][i] == my_role){
                printf("The winner is you\n");
            }else if(board[0][i] == friend_role && board[1][i] == friend_role && board[2][i] == friend_role){
                printf("The winner is Player2\n");
            }
        }
    
        //check diagonal
        if(board[0][0] == my_role && board[1][1] == my_role && board[2][2] == my_role){
            printf("The winner is you\n");
        }else if(board[0][0] == friend_role && board[1][1] == friend_role && board[2][2] == friend_role){
            printf("The winner is Player2\n");
        } 
    //}
    
}

void screen(){

    //row 1
    printf("--------------\n");
    printf("| %c | %c | %c |\n",board[0][0],board[0][1],board[0][2]);
    
    printf("| %c | %c | %c |\n",board[1][0],board[1][1],board[1][2]);
    
    printf("| %c | %c | %c |\n",board[2][0],board[2][1],board[2][2]);
    printf("--------------\n");

}






// plan
// 1 thrad screen
// 2 thread check_win
// 3 thread opensocket