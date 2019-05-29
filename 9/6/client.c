/*
 * Data:                2009-02-10
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc client.c -o client
 * Uruchamianie:        $ ./client <adres IP> <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>

#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

/* Generuje losowe iv
 *należy zwolnić później zaalokowaną pamięć poprzez free()
 */
unsigned char* generate_iv()
{
    srand(time(NULL));
    char* iv = (char*) malloc(16*sizeof(unsigned char));
    int i;
    for(i=0; i<16; i++)
    {
        iv[i] = rand()%(UCHAR_MAX+1);
    }

    return iv;
}

int main(int argc, char** argv) {

    int             sockfd;                 /* Desktryptor gniazda. */
    int             retval;                 /* Wartosc zwracana przez funkcje. */
    struct          sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    socklen_t       addr_len;               /* Rozmiar struktury w bajtach. */

    char            message[] = "Laboratorium PUS.";
    unsigned char   hmac_psk[] = "Klucz HMAC";
    unsigned char   cbc_psk[] = "Klucz CBC";


    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
    memset(&remote_addr, 0, sizeof(remote_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    remote_addr.sin_family = AF_INET;

    /* Konwersja adresu IP z postaci kropkowo-dziesietnej: */
    retval = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid network address!\n");
        exit(EXIT_FAILURE);
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    remote_addr.sin_port = htons(atoi(argv[2])); /* Numer portu. */
    addr_len = sizeof(remote_addr); /* Rozmiar struktury adresowej w bajtach. */


    unsigned char* iv = generate_iv();

    ERR_load_crypto_strings();

    unsigned char *hmac = HMAC(EVP_md5(), hmac_psk, strlen(hmac_psk), (unsigned char*)message, strlen(message), NULL, NULL);

    char buff[256];
    int buff_len = 0;
    snprintf(buff, sizeof(buff), "%s\n%s", hmac, message);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    const EVP_CIPHER* cipher = EVP_aes_128_cbc();
    retval = EVP_EncryptInit_ex(ctx, cipher, NULL, cbc_psk, iv);
    if (!retval) 
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }
    EVP_CIPHER_CTX_set_padding(ctx, 8);

    buff_len = strlen(buff);

    unsigned char ciphertext[256];
    int ciphertext_len;

    /* Szyfrowanie: */
    retval = EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len, (unsigned char*)buff, buff_len);

    if (!retval) 
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    retval = EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &ciphertext_len);
    if (!retval)
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }


    EVP_CIPHER_CTX_cleanup(ctx);


    fprintf(stdout, "Sending message to %s.\n", argv[1]);

    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
    retval = sendto(
                 sockfd,
                 ciphertext, strlen(ciphertext),
                 0,
                 (struct sockaddr*)&remote_addr, addr_len
             );

    fprintf(stdout, "Sending iv to %s.\n", argv[1]);

    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
    retval = sendto(
                 sockfd,
                 iv, 16*sizeof(unsigned char),
                 0,
                 (struct sockaddr*)&remote_addr, addr_len
             );



    free(iv);
    ERR_free_strings();




    if (retval == -1) {
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}
