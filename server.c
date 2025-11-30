#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define MAX_SIZE 1024

char* generate_response(char* path){
    char* buffer = malloc(MAX_SIZE);
    struct stat st;
    
    size_t file_size = 0; 
    if (stat(path, &st) == 0) {
        size_t file_size = st.st_size;
    }

    //open the file
    FILE* file = fopen(path, "r");
    
    //assign correct code
    int code = 200;
    char* response_code = "200 OK";
    if (file == NULL){
        code = 404; 
        response_code = "404 File Not Found";
    }
         
    printf("Code: %d\n", code);
    
    //gets contents of the file
    
    char* html = malloc(file_size);
    if (code == 200) {
        fread(html, 1, file_size, file); 
    }

    printf("Length: %d\n", (int) file_size);

    int c = snprintf(buffer, MAX_SIZE, "HTTP/1.1 %s\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n %s", response_code, (int) file_size, html);
    free(html);

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
        //we must remove the '/' from the path to be able to open it
        if (path[0] == '/') {
            path++;
        }
        printf("Path: %s\n", path);

        
        char* protocol = strtok(NULL, " ");
        printf("Protocol: %s\n", protocol);

        
        

        char* response = generate_response(path);
        if (response){
            write(client_fd, response, strlen(response));
            free(response);
        }
    }
     
    close(client_fd);
    close(server_fd);
    return 0;
}
