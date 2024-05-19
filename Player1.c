#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <gtk/gtk.h> 
#include <pthread.h>

void *socket_server();
void *socket_client();
void screen();
void send_data(GtkWidget *button, gpointer user_data);
void recv_data();
void update_board(int index,char c,GtkWidget *button,char *label);
void check_winner();
void *display();
void switch_to_game_mode(GtkWidget *window);
void create_grid_and_buttons(GtkWidget *window);
void create_server(GtkWidget *button, gpointer window);
void create_join(GtkWidget *button, gpointer window);
void join_server(GtkWidget *button, gpointer window);
void restart();
gboolean update_button_label(gpointer data);
void schedule_button_label_update(GtkWidget *button, const gchar *label);
void switch_to_game_mode(GtkWidget *window);
void create_grid_and_buttons(GtkWidget *window);



char board[3][3] = {{' ',' ',' '},
                    {' ',' ',' '},
                    {' ',' ',' '}}; //initial board

GtkWidget *buttons[9];
GtkWidget *ip_entry ;
char my_role;
char friend_role;
pthread_mutex_t lock_board;
// config connection
int connect_soc = 0;
char buffer[7];
char mode[10] = "";
GtkWidget *winner;
gchar *ip_host = "";

int main(int argc ,char *argv[]){
    
    pthread_t display_thread, game_thread;
    pthread_create(&display_thread, NULL, display, NULL);
    printf("Enter an mode(server or client): ");
    
    
    //wait for selecting mode
    while(strcmp(mode,"") == 0){};

    if ( strcmp(mode,"server") == 0){
        printf("server\n");
        socket_server(5000);
        //pthread_create(&game_thread, NULL, check_winner, NULL);
    }else if (strcmp(mode,"client") == 0){
        printf("client\n");
        //wait for entering ip 
        while(strcmp(ip_host,"") == 0){};
        //printf("%s\n",ip_host);
        socket_client();

    }else{
        printf("wrong mode\n");
    }
    return 0;
}


// create socket
void *socket_server(){
    int listen_server = 0,conn_cli = 0;
    struct sockaddr_in servAddr;
    memset(&servAddr, '0', sizeof(servAddr));
    memset(buffer, '0', sizeof(buffer)); 
    listen_server = socket(AF_INET, SOCK_STREAM, 0); // use TCP connection 

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(5000); 

    bind(listen_server, (struct sockaddr*)&servAddr, sizeof(servAddr)); 

    listen(listen_server, 1); 

    conn_cli = accept(listen_server, (struct sockaddr*)NULL, NULL);
    
     
    my_role = 'X';
    friend_role = 'O';
    connect_soc = conn_cli;

    while(conn_cli >= 0){
        
        printf("Your turn\n");
        recv_data();
        check_winner();
        memset(buffer, '0', sizeof(buffer)); 
        screen();
        
    }
    close(conn_cli);
}

void *socket_client(){
    int listen_server = 0,conn_cli = 0;
    struct sockaddr_in servAddr;
    memset(&servAddr, '0', sizeof(servAddr));
    memset(buffer, '0', sizeof(buffer)); 

    listen_server = socket(AF_INET, SOCK_STREAM, 0); // use TCP connection 

    servAddr.sin_family = AF_INET; // internet protocol IPV4
    servAddr.sin_port = htons(5000); 
    inet_pton(AF_INET, ip_host, &servAddr.sin_addr);
    conn_cli = connect(listen_server, (struct sockaddr *)&servAddr, sizeof(servAddr));
    
    if(conn_cli < 0){
        printf("no connection");
    }
    my_role = 'O';
    friend_role = 'X';
    connect_soc = listen_server;
    while(conn_cli >= 0){

        recv_data();
        check_winner();
        memset(buffer, '0', sizeof(buffer)); 
        printf("Your turn\n");
        screen();
        
        
    }

}

void send_data(GtkWidget *button, gpointer user_data){
    
    int *position = (int *)user_data;
    char label_send[2];
    sprintf(label_send, "%c", my_role);
    int row = position[0];
    int col = position[1];
    snprintf(buffer, sizeof(buffer),"%d\n",row * 3 + col + 1);
    write(connect_soc, buffer, strlen(buffer)); 
    update_board(row * 3 + col + 1, my_role, button , label_send);
    check_winner();
}

void recv_data(){
    
    char position[10] ;
    char label_recv[2];
    sprintf(label_recv, "%c", friend_role); 
    recv(connect_soc, buffer, sizeof(buffer), 0); 
    strcpy(position, buffer);
    printf("%s\n",position);
    if (atoi(position) == 111){
        char new_board[3][3] = {
        {' ', ' ', ' '},
        {' ', ' ', ' '},
        {' ', ' ', ' '}
        };
        gtk_label_set_text(GTK_LABEL(winner), "");
        memcpy(board, new_board, sizeof(new_board));

        for(int i = 0 ;i<9;i++){
        schedule_button_label_update(buttons[i] , " ");
        }
        
    }else{
        GtkWidget *button = buttons[atoi(position)-1];
        update_board(atoi(position), friend_role, button , label_recv);
    }
    
    
}

void update_board(int index,char c,GtkWidget *button,char label[]){
    
    pthread_mutex_lock (&lock_board);
  
    if (index > 0 && index < 4){
        if(board[0][index-1] == my_role || board[0][index-1] == friend_role ){
            printf("duplicate\n");
        }else{
            board[0][index-1] = c;
            schedule_button_label_update(button , label);
        }
    }else if (index >= 4 && index < 7){       
        if(board[1][index-4] == my_role || board[1][index-4] == friend_role ){
            printf("duplicate\n");
        }else{
            board[1][index-4] = c;
            schedule_button_label_update(button , label);
        }
    }else if (index >= 7 && index < 10){
        if(board[2][index-7] == my_role || board[2][index-7] == friend_role ){
            printf("duplicate\n");
        }else{
            board[2][index-7] = c;
            schedule_button_label_update(button , label);
        }
    }else{
        printf("wrong place\n");
    }
    pthread_mutex_unlock (&lock_board);
   
}
// update label
typedef struct {
    GtkWidget *button;
    gchar *label;
} UpdateData;

gboolean update_button_label(gpointer data) {
    UpdateData *update_data = (UpdateData *)data;
    gtk_button_set_label(GTK_BUTTON(update_data->button), update_data->label);
    g_free(update_data->label);
    g_free(update_data);
    return G_SOURCE_REMOVE;
}

void schedule_button_label_update(GtkWidget *button, const gchar *label) {
    UpdateData *update_data = g_new(UpdateData, 1);
    update_data->button = button;
    update_data->label = g_strdup(label);
    g_idle_add(update_button_label, update_data);
}

void check_winner(){
    pthread_mutex_lock (&lock_board);
   
    // check row
    for(int i = 0;i<3;i++){
        if(board[i][0] == my_role && board[i][1] == my_role && board[i][2] == my_role ){
            printf("YOU WIN\n");
            gtk_label_set_text(GTK_LABEL(winner), "YOU WIN");
        }else if(board[i][0] == friend_role && board[i][1] == friend_role && board[i][2] == friend_role){
            printf("YOU LOSE\n");
            gtk_label_set_text(GTK_LABEL(winner), "YOU LOSE");
        }
    }
    
    //check column
    for(int i = 0;i<3;i++){
        if(board[0][i] == my_role && board[1][i] == my_role && board[2][i] == my_role){
            printf("YOU WIN\n");
            gtk_label_set_text(GTK_LABEL(winner), "YOU WIN");
        }else if(board[0][i] == friend_role && board[1][i] == friend_role && board[2][i] == friend_role){
            printf("YOU LOSE\n");
            gtk_label_set_text(GTK_LABEL(winner), "YOU LOSE");
        }
    }

    //check diagonal
    if(board[0][0] == my_role && board[1][1] == my_role && board[2][2] == my_role){
        printf("YOU WIN\n");
        gtk_label_set_text(GTK_LABEL(winner), "YOU WIN");
    }else if(board[0][0] == friend_role && board[1][1] == friend_role && board[2][2] == friend_role){
        printf("YOU LOSE\n");
        gtk_label_set_text(GTK_LABEL(winner), "YOU LOSE");
    }if(board[0][2] == my_role && board[1][1] == my_role && board[2][0] == my_role){
        printf("YOU WIN\n");
        gtk_label_set_text(GTK_LABEL(winner), "YOU WIN");
    }else if(board[0][2] == friend_role && board[1][1] == friend_role && board[2][0] == friend_role){
        printf("YOU LOSE\n");
        gtk_label_set_text(GTK_LABEL(winner), "YOU LOSE");
    }
    
    pthread_mutex_unlock (&lock_board);
    
}

void screen(){

    //row 1
    printf("--------------\n");
    printf("| %c | %c | %c |\n",board[0][0],board[0][1],board[0][2]);
    
    printf("| %c | %c | %c |\n",board[1][0],board[1][1],board[1][2]);
    
    printf("| %c | %c | %c |\n",board[2][0],board[2][1],board[2][2]);
    printf("--------------\n");

}

void destroy(GtkWidget* widget, gpointer data) 
{ 
	gtk_main_quit(); 
    exit(EXIT_SUCCESS);
} 

void restart(){
    char new_board[3][3] = {
        {' ', ' ', ' '},
        {' ', ' ', ' '},
        {' ', ' ', ' '}
    };
    for(int i = 0 ;i<9;i++){
        schedule_button_label_update(buttons[i] , " ");
    }
    gtk_label_set_text(GTK_LABEL(winner), "");
    memcpy(board, new_board, sizeof(new_board));
    memset(buffer, '0', sizeof(buffer)); 
    snprintf(buffer, sizeof(buffer), "%d\n",111);
    write(connect_soc, buffer, strlen(buffer)); 
}

void create_grid_and_buttons(GtkWidget *window) {
    // Clear any existing children of the window
    gtk_container_foreach(GTK_CONTAINER(window), (GtkCallback)gtk_widget_destroy, NULL);
    
    // Create a grid container
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    winner = gtk_label_new("");
    GtkWidget *grid = gtk_grid_new();
    
    //create a box
    gtk_box_pack_start(GTK_BOX(box), winner, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);
    // Create buttons and add them to the grid
    int k = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            GtkWidget* button = gtk_button_new_with_label(" "); 
            gtk_widget_set_size_request(button, 100, 100); 
            int *position = g_new(int, 2);
            position[0] = i;
            position[1] = j;
            g_signal_connect(button, "clicked", G_CALLBACK(send_data), position);
            gtk_grid_attach(GTK_GRID(grid), button, j, i, 1, 1);
            buttons[k++] = button;
        }
    }
    // Create the "Restart" button
    GtkWidget *restart_button = gtk_button_new_with_label("Restart");
    g_signal_connect(restart_button, "clicked", G_CALLBACK(restart), NULL);
    gtk_grid_attach(GTK_GRID(grid), restart_button, 1, 4, 1, 1);  
    
    
    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_widget_show_all(window);
}

// Function to switch to the game mode screen
void switch_to_game_mode(GtkWidget *window) {
    // Remove any existing widgets from the window
    gtk_container_foreach(GTK_CONTAINER(window), (GtkCallback)gtk_widget_destroy, NULL);
    
    // Create widgets for the game mode screen
    GtkWidget *game_label = gtk_label_new("Select Game Mode:");
    GtkWidget *mode1_button = gtk_button_new_with_label("Create server");
    GtkWidget *mode2_button = gtk_button_new_with_label("Join server");
    
    // Connect signals for mode selection
    g_signal_connect(mode1_button, "clicked", G_CALLBACK(create_server), window);
    g_signal_connect(mode2_button, "clicked", G_CALLBACK(create_join), window);
    
    // Create a vertical box to hold the widgets
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), game_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), mode1_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), mode2_button, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Show all widgets
    gtk_widget_show_all(window);
}

// Callback function for creating server
void create_server(GtkWidget *button, gpointer window) {

    strcpy(mode, "server");
    create_grid_and_buttons(GTK_WIDGET(window));
}

// Callback function for joing server
void join_server(GtkWidget *button, gpointer window) {
    
    ip_host = g_strdup(gtk_entry_get_text(GTK_ENTRY(ip_entry)));
    create_grid_and_buttons(GTK_WIDGET(window));
}


// Callback function for creating join server
void create_join(GtkWidget *button, gpointer window) {
    strcpy(mode, "client");
    // Clear any existing children of the window
    gtk_container_foreach(GTK_CONTAINER(window), (GtkCallback)gtk_widget_destroy, NULL);
    
    // Create box 
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    // widget
    ip_entry = gtk_entry_new();
    GtkWidget *enter_button = gtk_button_new_with_label("Join");
    GtkWidget *label = gtk_label_new("Enter IP address");

    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), ip_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), enter_button, FALSE, FALSE, 0);
    
    
    g_signal_connect(enter_button, "clicked", G_CALLBACK(join_server), window);

    
    // Show all widgets
    gtk_widget_show_all(window);
}


void *display(){

    // Initialize GTK
    gtk_init(NULL,NULL);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tic Tac Toe");
    
    g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 100); 
    // Start with the game mode selection screen
    switch_to_game_mode(window);
    
    // Show the window
    gtk_widget_show(window);
    
    // Run the GTK main loop
    gtk_main();
    
    return 0;
}
