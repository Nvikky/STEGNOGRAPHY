/*
Name : N venkata prasad
date : 29/09/24
decription : encode.c -- consists of encode operation function calls and definitions
*/
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "common.h"
#include "types.h"

/* Function Definitions */
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
        return e_encode;
    else if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    return e_unsupported;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //check if .bmp file is given or not
    if (strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    //secret file validate extension 
    char *temp = strstr(argv[3], ".");
    strcpy(encInfo->extn_secret_file, temp);
    if (strcmp(temp, ".txt") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else if (strcmp(temp, ".sh") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else if (strcmp(temp, ".c") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else
        return e_failure;
    //if 4th arg is given it checks for .bmp
    if (argv[4] != NULL)
    {
        if ((strcmp(strstr(argv[4], "."), ".bmp")) == 0)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
            return e_failure;
    }
    //if not given store default as stego.bmp
    else
        encInfo->stego_image_fname = "stego.bmp";

    return e_success;

}
Status open_files(EncodeInfo *encInfo)
{
    printf("INFO: Openning required files\n");
    //open source file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    //validate if NULL or not
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }
    else
    {
        printf("INFO: Opened Skeleton/beautiful.bmp\n");
    }
    //open secret file 
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    //validate if NULL or not
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }
    else
    {
        printf("Opened Secret.txt\n");
    }

    //open Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    //validate if NULL or not
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }
    else
    {
        printf("INFO: Opened Stego.bmp\n");
    }

    //if failure return e_success
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: Checking capacity to store data\n");
    if (check_capacity(encInfo) == e_success)
    {
        printf("INFO: Found OK\n");
        printf("INFO: Copying Image Header\n");
        if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
        {
            printf("INFO: Done\n");
            printf("INFO: Encoding Magic String\n");
            if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
            {
                printf("INFO: Done\n");
                printf("INFO: Copying Extention size");
                printf("%s\n", encInfo->extn_secret_file);
                if (encode_file_extn_size(encInfo->extn_secret_file, encInfo) == e_success)
                {
                    printf("INFO: Done\n");
                    printf("INFO: Copying Secret file extention\n");
                    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                    {
                        printf("INFO: Done\n");
                        printf("INFO: Copying Secret file size\n");
                        if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                        {
                            printf("INFO: DONE\n");
                            printf("INFO: Copying Secret file data\n");
                            if (encode_secret_file_data(encInfo) == e_success)
                            {
                                printf("INFO: DONE\n");
                                printf("INFO: Copying left over Data\n");
                                if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                {
                                    printf("INFO: DONE\n");
                                    return e_success;
                                }
                                else
                                {
                                    printf("ERROR: Remaining data not copied\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("ERROR: secret file data not copied\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("ERROR:Secret file size not copied\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("ERROR: Secret file extention is not stored.\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("ERROR: File extention not copied\n");
                    return e_failure;
                }
            }
            else
            {
                printf("ERROR: Magic string is not encoded\n");
                return e_failure;
            }
        }
        else
        {
            printf("ERROR: Copying image header is not done\n");
            return e_failure;
        }
    }
    else
    {
        printf("ERROR:Capicity is not enough to encode\n");
        return e_failure;
    }
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

Status check_capacity(EncodeInfo *encInfo)
{
    int len = strlen(MAGIC_STRING);
    // printf("Magic string length %d\n", len);
    //size of source image
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    //size of secret text file
    uint size_secret_file = get_file_size(encInfo->fptr_secret);
    encInfo->size_secret_file = size_secret_file;
    //calculate total encoding size needed
    uint encode_size = 54 + (len + 4 + 4 + strlen(encInfo->extn_secret_file) + size_secret_file) * 8;
    //check if image can store 
    if (encInfo->image_capacity > encode_size)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

uint get_file_size(FILE *fptr)
{
    // Move file pointer to the end of the file
    fseek(fptr, 0, SEEK_END);
    //stores the size of file
    return ftell(fptr);
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];
    //move the filepointer intial position of src image and destination image
    rewind(fptr_src_image);
    rewind(fptr_dest_image);
    //read 54 bytes from src to buffer
    fread(buffer, 54, 1, fptr_src_image);
    //write 54 bytes from buffer to destination 
    fwrite(buffer, 54, 1, fptr_dest_image);
    printf("INFO: Copying Image Header\n");
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    int magicstr_len = strlen(MAGIC_STRING);
    //function call to encode magic string
    encode_data_to_image(magic_string, magicstr_len, encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{

    char image_buffer[8];
    for (int i = 0; i < size; i++)
    {
        //read 8 bytes from sorce img to image_buffer
        fread(image_buffer, 8, 1, fptr_src_image);
        //function call to encode
        encode_byte_to_lsb(data[i], image_buffer);
        //write encoded image buffer to stego_image
        fwrite(image_buffer, 8, 1, fptr_stego_image);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char image_buffer[])
{
    for (int i = 0; i < 8; i++)
    {
        //encoding the byte into the image buffer
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7 - i)) & 1);
    }
    return e_success;
}

Status encode_file_extn_size(char *file_extn, EncodeInfo *encInfo)
{
    char image_buffer[32];
    //read 32 bytes from src to image buffer
    fread(image_buffer, 32, 1, encInfo->fptr_src_image);
    long size = strlen(file_extn);
    //call function  to encode and modify image buffer
    encode_size_to_lsb(size, image_buffer);
    //write modified image_buffer to stegoimage
    fwrite(image_buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_size_to_lsb(long size, char image_buffer[])
{
    // printf("Data size is %ld\n", size);
    for (int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size >> (31 - i)) & 1);
    }
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    //call function to encode 
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_size(uint size_secret_file, EncodeInfo *encInfo)
{
    char image_buffer[32];
    // Read 32 bytes from the source image file into image_buffer
    fread(image_buffer, 32, 1, encInfo->fptr_src_image);
    //call function to encode
    encode_size_to_lsb(size_secret_file, image_buffer);
    //write to stego_image
    fwrite(image_buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char sec_file[encInfo->size_secret_file];
    rewind(encInfo->fptr_secret);
    //make sure offset set to intial 
    fread(sec_file, encInfo->size_secret_file, 1, encInfo->fptr_secret);
    //read each byte from secret file call encode to image
    if (encode_data_to_image(sec_file, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        return e_success;
    }
    return e_failure;
}
//copy remaining data to stego_image
Status copy_remaining_img_data(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char ch;
    //byte to byte
    while (fread(&ch, 1, 1, fptr_src_image) != 0)
    {
        fwrite(&ch, 1, 1, fptr_stego_image);
    }
    return e_success;
}