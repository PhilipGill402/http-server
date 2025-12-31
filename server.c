#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define MAX_SIZE 1024

typedef struct response_t {
    char* html;
    int code;
    long long size;
} response_t;

void get_404_info(response_t* response) {
    struct stat st;
    long long file_size = 0;
    if (stat("404.html", &st) == 0) {
        file_size = (long long)st.st_size;
    } else {
        perror("file size");
        return;
    }
    
    char* html = malloc(file_size);
    FILE* file = fopen("404.html", "r");
    fread(html, sizeof(char), file_size, file);
    
    fclose(file);


    response->code = 404;
    response->size = file_size;
    response->html = html;
}

void get_response_info(response_t* response, char* path) {
    struct stat st;
    
    long long file_size = 0; 
    if (stat(path, &st) == 0) {
        file_size = (long long)st.st_size;
    } else {
        get_404_info(response);
        return;
    }

    response->size = file_size;

    //open the file
    FILE* file = fopen(path, "r");
    
    //assign correct code
    response->code = 200;

    //gets contents of the file (if file was not found then get contents of '404.html')
    char* html = malloc(file_size);
    response->html = malloc(file_size);
    fread(html, sizeof(char), file_size, file);
    strcpy(response->html, html);
    
    //cleanup
    free(html);
    html = NULL;
    fclose(file);
}

char* generate_response(const response_t* response){
    char* buffer = malloc(MAX_SIZE);

    char response_code[MAX_SIZE];

    switch(response->code) {
        case 200:
            strcpy(response_code, "200 OK\0");
            break;
        default:
            strcpy(response_code, "404 Not Found\0");
            break;
    }

    int c = snprintf(buffer, MAX_SIZE, "HTTP/1.1 %s\r\nContent-Type: text/html; charset=UTF8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n %s", response_code, (int) response->size, response->html);

    if (c < 0){
        perror("response");
        return NULL;
    }
    
    return buffer;
}

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
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
        //we must remove the '/' from the path to be able to open it
        if (path[0] == '/' && path[1] != '\0') {
            path++;
        } else if (path[0] == '/') {
            strcpy(path, "index.html");
        }
        printf("Path: %s\n", path);

        
        char* protocol = strtok(NULL, " ");
        printf("Protocol: %s\n", protocol);

        
        
        response_t* response = malloc(sizeof(response_t));
        get_response_info(response, path);

        printf("Code: %d\n", response->code);
        printf("Size: %d\n", (int)response->size);
        printf("HTML: %s\n", response->html);

        char* response_buffer = generate_response(response);
        printf("Response: %s\n", response_buffer);

        if (response_buffer){
            write(client_fd, response_buffer, strlen(response_buffer));
            
        }
        
        free(response->html);
        free(response_buffer);
        free(response);
        response_buffer = NULL;
        response = NULL;
    }
     
    close(client_fd);
    close(server_fd);
    return 0;
}
