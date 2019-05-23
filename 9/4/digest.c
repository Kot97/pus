/*
 * Data:                2009-05-05
 * Data modyfikacji:    2019-05-23
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Zmodyfikował:        Marcin Kurdziel <https://github.com/Kot97>
 * Kompilacja:          clang -o digest digest.c -lcrypto 
 * Uruchamianie:        ./digest <nazwa funkcji>
 *                      ./digest <nazwa funkcji> < NAZWA_PLIKU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/evp.h>

int main(int argc, char **argv) 
{
    /* Wartosc zwracana przez funkcje: */
    int retval;

    /* Wiadomosc: */
    char message[64];

    /* Skrot wiadomosci: */
    unsigned char digest[EVP_MAX_MD_SIZE];

    /* Rozmiar tekstu i szyfrogramu: */
    unsigned int message_len, digest_len;

    const EVP_MD* md;

    if (argc != 2) 
    {
        fprintf(stderr, "Invocation: %s <DIGEST NAME>\n", argv[0]);
        return 1;
    }

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /*
     * Zaladowanie nazw funkcji skrotu do pamieci.
     * Wymagane przez EVP_get_digestbyname():
     */
    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname(argv[1]);
    if (!md) 
    {
        fprintf(stderr, "Unknown message digest: %s\n", argv[1]);
        return 1;
    }

    /* Pobranie maksymalnie 64 znakow ze standardowego wejscia: */
    if (fgets(message, 64, stdin) == NULL) 
    {
        fprintf(stderr, "fgets() failed!\n");
        return 1;
    }

    message_len = strlen(message);

    /* Kontekst: */
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    /* Inicjalizacja kontekstu: */
    EVP_MD_CTX_init(ctx);

    /* Parametry funkcji skrotu: */
    fprintf(stdout, "Digest parameters:\n");
    fprintf(stdout, "Block size: %d bits\n", EVP_MD_block_size(md));
    fprintf(stdout, "Digest size: %d bytes\n\n", EVP_MD_size(md));

    /* Konfiguracja kontekstu: */
    retval = EVP_DigestInit_ex(ctx, md, NULL);
    if (!retval) 
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    /* Obliczenie skrotu: */
    retval = EVP_DigestUpdate(ctx, message, message_len);
    if (!retval) 
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    /*Zapisanie skrotu w buforze 'digest': */
    retval = EVP_DigestFinal_ex(ctx, digest, &digest_len);
    if (!retval) 
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    /*
     * Usuwa wszystkie informacje z kontekstu i zwalnia pamiec zwiazana
     * z kontekstem:
     */
    EVP_MD_CTX_free(ctx);

    /* Usuniecie nazw funkcji skrotu z pamieci. */
    EVP_cleanup();

    fprintf(stdout, "Digest (hex): ");
    for (int i = 0; i < digest_len; ++i) fprintf(stdout, "%02x", digest[i]);

    fprintf(stdout, "\n");

    if (ctx) free(ctx);

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    exit(EXIT_SUCCESS);
}
