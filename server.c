#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define MAX_SIZE 1024

int main(){
    int server_fd;
    struct sockaddr_in server_addr;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;
    
    if (server_fd < 0){
        perror("socket");
        return -1;
    }
    
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        return -1;
    }
    

    if (listen(server_fd, 10) < 0){
        perror("listen");
        return -1;
    }
    
    while (1){
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd < 0){
            perror("accept");
            continue; 
        }

        char buffer[MAX_SIZE];
        int bytes_read = read(client_fd, buffer, MAX_SIZE-1);
        buffer[bytes_read] = '\0';
        printf("Full Request:\n%s\n", buffer);

        char* first_line = strtok(buffer, "\r\n");

        char* method = strtok(first_line, " ");
        printf("HTTP Method: %s\n", method);

        char* path = strtok(NULL, " ");
        printf("Path: %s\n", path);
        
        char* protocol = strtok(NULL, " ");
        printf("Protocol: %s\n", protocol);

        FILE* file = fopen(path, "w");
        if (file == NULL){
            write(client_fd, "404 Error: File Not Found\n", 26);
        }
    }
     
    close(client_fd);
    close(server_fd);
    printf("Hello World\n");
    return 0;
}
