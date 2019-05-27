/*
 * Data:                2019-05-27
 * Autor:               Marcin Kurdziel <https://github.com/Kot97>
 * Kompilacja:          clang -o mac mac.c -lssl -lcrypto 
 * Uruchamianie:        ./mac 
 *                      ./mac < NAZWA_PLIKU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/hmac.h>

int main(int argc, char **argv) 
{
    /* Wartosc zwracana przez funkcje: */
    int retval;

    /* Wiadomosc: */
    char message[64];

    /* Rozmiar tekstu i szyfrogramu: */
    unsigned int message_len, digest_len;


    if (argc != 2) 
    {
        fprintf(stderr, "Invocation: %s <DIGEST NAME>\n", argv[0]);
        return 1;
    }

    /* Klucz: */
    unsigned char key[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                           0x00,0x01,0x02,0x03,0x04,0x05};

    /* Wektor inicjalizacyjny: */
    unsigned char iv[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                          0x00,0x01,0x02,0x03,0x04,0x05};

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /* Pobranie maksymalnie 64 znakow ze standardowego wejscia: */
    if (fgets(message, 64, stdin) == NULL) 
    {
        fprintf(stderr, "fgets() failed!\n");
        return 1;
    }

    unsigned char *digest = HMAC(EVP_sha1(), key, strlen(key), (unsigned char*)message, strlen(message), NULL, NULL);

    char mdString[20];
    for(int i = 0; i < 10; i++) sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
 
    printf("HMAC digest: %s\n", mdString);

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    exit(EXIT_SUCCESS);
}
