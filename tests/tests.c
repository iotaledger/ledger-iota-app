/*
Copyright 2021   Thomas Pototschnig <microengineer18@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.



You need a running simulator for the tests. Instructions e.g. here:

    https://gitlab.com/ledger-iota-chrysalis/ledger-iota-app-docker

*/
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct {
    uint8_t *data;
    size_t data_len;
    size_t read_index;
} stream_t;

static stream_t data_stream;

void get_data(uint8_t *dst, size_t dst_len, size_t max_len)
{
    if (data_stream.read_index + dst_len > data_stream.data_len) {
        printf("invalid copy!\n");
        exit(1);
    }
    if (dst_len > max_len) {
        printf("destination too small\n");
        exit(1);
    }
    memcpy(dst, &data_stream.data[data_stream.read_index], dst_len);
    data_stream.read_index += dst_len;
}


uint32_t le2be(uint32_t v)
{
    return (v & 0xff000000) >> 24 | (v & 0x00ff0000) >> 8 |
           (v & 0x0000ff00) << 8 | (v & 0x000000ff) << 24;
}

uint32_t be2le(uint32_t v)
{
    return le2be(v);
}

void dump(uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (!(i % 16) && i != 0) {
            printf("\n");
        }
        printf("%02x ", data[i]);
    }
    printf("\n");
}

// patch the version in the reference bin files
// this is done to be able to use older testfiles with newer compilations to 
// find degradations in the code. When generating new testfiles after compiling, 
// testfiles could be wrong and this possibly wouldn't be noticed.
void patch_app_version(uint8_t *command, uint8_t command_len, uint8_t *answer, uint8_t answer_len, uint8_t* buffer) {
    const uint8_t cmd_get_app_config[5] = {0x7b, 0x10, 0x00, 0x00, 0x00};
 
    if (command_len == 5 && answer_len == 8) {
        if (!memcmp(command, cmd_get_app_config, command_len)) {
            // accept any app version
            buffer[0] = answer[0];
            buffer[1] = answer[1];
            buffer[2] = answer[2];
            buffer[3] = answer[3] = 0;  // flags that is unused; was uninitialized before app 0.6.3
        }
    }
}

void replay(int sockfd, uint8_t *data, size_t data_len)
{
    int lastperc = 0;

    data_stream.data = data;
    data_stream.data_len = data_len;
    data_stream.read_index = 0;

    while (data_stream.data_len - data_stream.read_index) {
        uint8_t command_buffer[256] = {0};
        uint8_t answer_buffer[256] = {0};
        uint8_t buffer[256] = {0};

        uint32_t command_len;
        get_data((uint8_t *)&command_len, sizeof(uint32_t), sizeof(uint32_t));
        get_data(command_buffer, command_len, sizeof(command_buffer));

        uint32_t answer_len;
        get_data((uint8_t *)&answer_len, sizeof(uint32_t), sizeof(uint32_t));
        if (answer_len > sizeof(answer_buffer)) {
            printf("answer buffer too small\n");
            exit(1);
        }
        get_data(answer_buffer, answer_len, sizeof(answer_buffer));

        uint32_t speculos_len = le2be(command_len);
        write(sockfd, &speculos_len, sizeof(uint32_t));
        write(sockfd, command_buffer, command_len);

        //        dump((uint8_t*) &speculos_len, sizeof(uint32_t));
        //        dump((uint8_t*) command_buffer, command_len);

        read(sockfd, &speculos_len, sizeof(uint32_t));
        //        dump((uint8_t*) &speculos_len, sizeof(uint32_t));

        speculos_len = be2le(speculos_len) + 2;
        read(sockfd, buffer, speculos_len);

        if (speculos_len != answer_len) {
            printf("answer size mismatch!\n");
            exit(1);
        }

        //        dump((uint8_t*) buffer, answer_len);

        patch_app_version(command_buffer, command_len, answer_buffer, answer_len, buffer);

        if (memcmp(buffer, answer_buffer, answer_len)) {
            printf("data mismatch!\n");
            printf("request:  "); dump((uint8_t*) command_buffer, command_len);
            printf("response: "); dump((uint8_t*) answer_buffer, answer_len);
            printf("is:       "); dump((uint8_t*) buffer, answer_len);

            exit(1);
        }

        float perc = (float)data_stream.read_index /
                     (float)data_stream.data_len * 100.0f;
        if ((int)perc != lastperc) {
            printf("%d%% done ...\n", (int)perc);
            lastperc = (int)perc;
        }
    }
}

uint8_t *read_file(const char *filename, size_t *filesize)
{
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("error opening file %s\n", filename);
        exit(1);
    }

    fseek(f, 0l, SEEK_END);
    *filesize = ftell(f);
    rewind(f);

    uint8_t *data = (uint8_t *)malloc(*filesize);
    size_t read = fread(data, sizeof(uint8_t), *filesize, f);
    if (read != *filesize) {
        printf("error reading from file!\n");
        exit(1);
    }
    return data;
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        printf("usage: %s filename.bin\n", argv[0]);
        exit(1);
    }

    const char *filename = argv[1];
    size_t filesize;
    uint8_t *data = read_file(filename, &filesize);


    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(1);
    }
    else {
        printf("Socket successfully created..\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(9999);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(1);
    }
    else {
        printf("connected to the server..\n");
    }

    // function for chat
    replay(sockfd, data, filesize);

    // close the socket
    close(sockfd);

    free(data);
    return 0;
}