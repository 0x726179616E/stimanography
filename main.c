#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdbool.h> 
#include <time.h> 

/* assume the image header is 1024 bytes long */
#define IMAGE_HEADER_SIZE 1024 
/* assuming a 1024x1024 image */
#define IMAGE_SIZE 1024*1024 
/* max message length in bytes */
#define MAX_MESSAGE_SIZE (IMAGE_SIZE - IMAGE_HEADER_SIZE) / 8 

void encrypt_message(const char* image_path, const char* output_path, const char* message);
void decrypt_message(const char* image_path);

int main(int argc, char *argv[]) {
    /* requires at least one flag and the input image path for encryption */
    if (argc < 3) {
        fprintf(stderr, "usage for encryption: %s [-e] [-m message] [-o output image path] [input image path]\n", argv[0]);
        fprintf(stderr, "usage for decryption: %s [-d] [input image path]\n", argv[0]);
        return 1;
    }

    bool encrypt_mode = false;
    bool decrypt_mode = false;
    char *message = NULL;
    char *output_path = NULL;
    char *input_path = NULL;

    /* parse commandline arguments */
    for (int i = 1; i < argc - 1; i++) { 
        if (strcmp(argv[i], "-h") == 0) {
            fprintf(stderr, "usage for encryption: %s [-e] [-m message] [-o output image path] [input image path]\n", argv[0]);
            fprintf(stderr, "usage for decryption: %s [-d] [input image path]\n", argv[0]);
            return 0;
        } else  if (strcmp(argv[i], "-e") == 0) {
            encrypt_mode = true;
        } else if (strcmp(argv[i], "-d") == 0) {
            decrypt_mode = true;
        } else if (strcmp(argv[i], "-m") == 0 && (i + 1 < argc)) {
            message = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && (i + 1 < argc)) {
            output_path = argv[++i];
        }
    }
    /* last argument is reserved for the input path */
    input_path = argv[argc - 1]; 

    if (encrypt_mode && decrypt_mode) {
        fprintf(stderr, "usage error: specify either encryption [-e] or decryption [-o] mode, cannot operate both simulatenously.\n");
        return 1;
    }

    if (encrypt_mode) {
        if (!message || !output_path) {
            fprintf(stderr, "usage error: missing arguments for encryption mode: message [-m] and output image path [-o] are required.\n");
            return 1;
        }
        encrypt_message(input_path, output_path, message);
    } else if (decrypt_mode) {
        decrypt_message(input_path);
    } else {
        fprintf(stderr, "usage error: no mode specified, use [-e] for encryption or [-d] for decryption.\n");
        return 1;
    }
    return 0;
}

void encrypt_message(const char* image_path, const char* output_path, const char* message) {
    FILE *img = fopen(image_path, "rb");
    FILE *output_img = fopen(output_path, "wb");
    if (!img) {
        fprintf(stderr, "encryption error: can't open input image file\n");
        exit(1);
    } else if (!output_img) {
        fprintf(stderr, "encryption error: can't open output image file\n");
        exit(1);
    }

    size_t message_length = strlen(message) + 1;
    if (message_length > MAX_MESSAGE_SIZE) {
        fprintf(stderr, "encryption error: message too long to encrypt in the image\n");
        fclose(img);
        fclose(output_img);
        exit(1);
    }

    unsigned char buffer;
    size_t message_index = 0;
    size_t bit_index = 0;

    /* seed random number generator */
    srand((unsigned) time(NULL));

    /* copy image header - assuming raw greyscale bytes start immediately */
    for (int i = 0; i < IMAGE_HEADER_SIZE; ++i) { /* dummy header handling */
        if (fread(&buffer, 1, 1, img) != 1) break;
        fwrite(&buffer, 1, 1, output_img);
    }

    /* process each byte of the image */
    size_t bytes_processed = IMAGE_HEADER_SIZE;

    /* track whether dilineating newlines have been added after the message */
    bool added_nl = false;

    while (fread(&buffer, 1, 1, img) == 1) {
        if (message_index < message_length - 1) {
            /* clear the least significant bit (LSB) */
            unsigned char bit = (message[message_index] >> bit_index) & 1;
            /* embed LSB with message bit */
            buffer = (buffer & 0xFE) | bit;
            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                message_index++;
            }
        } else if (!added_nl) {
            static int newlines_added = 0;
            unsigned char newline_bit = (0x0A >> newlines_added % 8) & 1; 
            buffer = (buffer & 0xFE) | newline_bit;
            newlines_added++;
            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                if (newlines_added == 16) { /* 8 bits per newline * 2 newlines */
                    added_nl = true;
                }
            }
        } else {
            /* pad the LSB with random data */
            unsigned char random_bit = rand() % 2;
            buffer = (buffer & 0xFE) | random_bit; 
        }
        fwrite(&buffer, 1, 1, output_img);
        bytes_processed++;
    }

    if (bytes_processed < IMAGE_SIZE) {
        fprintf(stderr, "encryption error; image file is smaller than expected\n");
        fclose(img);
        fclose(output_img);
        exit(1);
    }
    fprintf(stdout, "\nsuccessfully encrypted message to: %s\n", output_path);
    fclose(img);
    fclose(output_img);
}

void decrypt_message(const char* image_path) {
    FILE *img = fopen(image_path, "rb");
    if (!img) {
        fprintf(stderr, "decryption error: can't open input image file\n");
        exit(1);
    }

    unsigned char buffer;
    unsigned char message[MAX_MESSAGE_SIZE];
    memset(message, 0, SEEK_SET);

    size_t byte_index = 0;
    size_t bit_index = 0;

    /* skip image header */
    fseek(img, 1024, SEEK_SET);
    /* extract message */
    while (fread(&buffer, 1, 1, img) == 1 && byte_index < MAX_MESSAGE_SIZE) {
        message[byte_index] |= (buffer & 1) << bit_index;
        bit_index++;
        if (bit_index == 8) {
            bit_index = 0;
            byte_index++;
        }
    }
    fprintf(stdout, "\nsuccessfully decrypted message:\n%s\n", message);
    fclose(img);
}
