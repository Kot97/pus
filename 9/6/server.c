/*
 * Data:                2009-02-10
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc server.c -o server
 * Uruchamianie:        $ ./server <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>

#include <stdlib.h>
// #include <time.h>
// #include <limits.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

int main(int argc, char** argv) {

    int             sockfd; /* Deskryptor gniazda. */
    int             retval; /* Wartosc zwracana przez funkcje. */

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct          sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t       client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez recvfrom(): */
    char            buff[256];

    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char            addr_buff[256];


    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family          =       AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr     =       htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port            =       htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len                 =       sizeof(server_addr);

    /* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(sockfd, (struct sockaddr*) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is waiting for UDP datagram...\n");

    client_addr_len = sizeof(client_addr);

    /* Oczekiwanie na dane od klienta: */
    retval = recvfrom(
                 sockfd,
                 buff, sizeof(buff),
                 0,
                 (struct sockaddr*)&client_addr, &client_addr_len
             );
    if (retval == -1) {
        perror("recvfrom()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "UDP datagram received from %s:%d.\n",
            inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
            ntohs(client_addr.sin_port)
           );



    // unsigned char iv[20];

    // /* Oczekiwanie na dane od klienta: */
    // retval = recvfrom(
    //              sockfd,
    //              iv, sizeof(iv),
    //              0,
    //              (struct sockaddr*)&client_addr, &client_addr_len
    //          );
    // if (retval == -1) {
    //     perror("recvfrom()");
    //     exit(EXIT_FAILURE);
    // }

    // fprintf(stdout, "UDP datagram with iv received from %s:%d.\n",
    //         inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
    //         ntohs(client_addr.sin_port)
    //        );

    unsigned char iv[] = "123456789asdfghj";

    



    ERR_load_crypto_strings();

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    const EVP_CIPHER* cipher = EVP_aes_128_cbc();
    unsigned char   cbc_psk[] = "Klucz CBC";

    retval = EVP_DecryptInit_ex(ctx, cipher, NULL, cbc_psk, iv);
    if (!retval) 
    {
        fprintf(stderr, "DecryptInit\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }

    /*
     * Domyslnie OpenSSL stosuje padding. 0 - padding nie bedzie stosowany.
     * Funkcja moze byc wywolana tylko po konfiguracji kontekstu
     * dla szyfrowania/deszyfrowania (odzielnie dla kazdej operacji).
     */
    EVP_CIPHER_CTX_set_padding(ctx, 8);
    int bufflen = strlen(buff);
    char message[256];
    int messagelen = 0;

    /* Odszyfrowywanie: */
    retval = EVP_DecryptUpdate(ctx, (unsigned char*)message, &messagelen, buff, bufflen);

    if (!retval) 
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    /*
     * Prosze zwrocic uwage, ze rozmiar bufora 'plaintext' musi byc co najmniej o
     * rozmiar bloku wiekszy od dlugosci szyfrogramu (na padding):
     */
    int tmp = 0;
    retval = EVP_DecryptFinal_ex(ctx, (unsigned char*)message + messagelen, &tmp);
    if (!retval) 
    {
        fprintf(stderr, "DecryptFinal\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }

    messagelen += tmp;
    message[messagelen] = '\0';

    char* plaintext;
    int plaintextlen;
    unsigned char* hmac;
    int hmaclen;

    hmac = strtok(buff, "\n");
    plaintext = strtok(NULL, "\n");
    plaintextlen = strlen(plaintext);
    hmaclen = strlen(hmac);

    unsigned char   hmac_psk[] = "Klucz HMAC";
    unsigned char* hmac2 = HMAC(EVP_md5(), hmac_psk, strlen(hmac_psk), (unsigned char*)plaintext, strlen(plaintext), NULL, NULL);

    int hmac_compare_result = strcmp(hmac, hmac2);

    EVP_CIPHER_CTX_cleanup(ctx);
    ERR_free_strings();

    printf("HMAC: %s\n", (hmac_compare_result==0) ? "Correct" : "Incorrect");
    printf("Message: %s\n", plaintext);

    close(sockfd);
    exit(EXIT_SUCCESS);
}
