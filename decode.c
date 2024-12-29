/*
Name : N venkata prasad
date : 29/09/24
decription : decode.c -- consists of decode operation function calls and defintions
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "decode.h"

//for decoding 3rd argument is not compulsory else cretes a default file 
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    decInfo->decode_fname=malloc(20);
    //top check if .bmp is given or not
    if(strstr(argv[2],".bmp") != NULL)
    {
        decInfo->stego_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    //if arg[3] is present store in fname
    if (argv[3] != NULL)
    {
        decInfo->decode_fname = argv[3];
    }
    // store as default 
    else
    {
        strcpy(decInfo->decode_fname, "decode");
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("Decoding started\n");
    printf("INFO: Openning required files\n");
    if (open_decode_files(decInfo) == e_success)
    {
        printf("INFO: Opened .bmp file\n");
        printf("INFO: Decoding Magic String Signature\n");
        if (decode_magic_string(decInfo) == e_success)
        {
            printf("INFO: DONE\n");
            if (decode_secret_file_extn_size(decInfo) == e_success)
            {
                printf("INFO: Decoding Output File Extenstion\n");
                if (decode_secret_extn(decInfo) == e_success)
                {
                    printf("INFO: Done\n");
                    printf("INFO: Decoding File Size\n");
                    if (decode_secret_file_size(decInfo) == e_success)
                    {
                        printf("INFO: Done\n");
                        printf("INFO: Decoding File Data\n");
                        if (decode_secret_file_data(decInfo) == e_success)
                        {
                            printf("INFO: Done\n");
                            printf("INFO: Decoding Done Successfully\n");
                            return e_success;
                        }
                        printf("ERROR: Failed to copy secret file data\n");
                        return e_failure;
                    }
                    printf("ERROR: Failed to copy secret file size\n ");
                    return e_failure;
                }
                printf("ERROR: Failed to copy secret file extention\n");
                return e_failure;
            }
            printf("ERROR: Failed to copy secret file extention size\n");
            return e_failure;
        }
        printf("ERROR : Failed to decode magic string\n");
        return e_failure;
    }
    printf("ERROR: Failed to open decoding files\n");
    return e_failure;
}

//open the required files
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego = fopen(decInfo->stego_fname, "r");
    //validate if given .bmp file is exist or not
    if (decInfo->fptr_stego == NULL)
    {
        printf("ERROR: Missing image.bmp file\n");
        return e_failure;
    }
    return e_success;
}

char decode_lsb_to_char(DecodeInfo *decInfo)
{
    // Buffer to store 8 bytes 
    char buffer[8];
    // Read 8 bytes from the stego file into the buffer
    fread(buffer, 8, 1, decInfo->fptr_stego);
    char ch = 0;
    // Loop through the 8 bytes to extract the LSB from each byte
    for (int i = 0; i < 8; i++)
    {
        // Extract the least significant bit (LSB) or with ch
        ch = ch | (buffer[i] & 1);
        if (i != 7)
            ch = ch << 1;
    }
    return ch;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    int magic_len = strlen(MAGIC_STRING);
     // Move the file pointer to the position to skip bmp header
    fseek(decInfo->fptr_stego, 54, SEEK_SET);
    char magic_str[5];
    int i;
    //decode till magic_len
    for (i = 0; i < magic_len; i++)
    {
        magic_str[i] = decode_lsb_to_char(decInfo);
    }
    magic_str[i] = '\0';
    // Compare the decoded magic string with the expected MAGIC_STRING
    if (strcmp(magic_str, MAGIC_STRING) == 0)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

int decode_lsb_to_int(DecodeInfo *decInfo)
{
    //int = 4 => 4*8 ==32
    char intbuffer[32];
    int size = 0;
    fread(intbuffer, 32, 1, decInfo->fptr_stego);
    
    // Loop through the 32 bytes to extract the LSB from each byte
    for (int i = 0; i < 32; i++)
    {
        size = size | (intbuffer[i] & 1);
        //no need to shift if it is last bit
        if (i != 31)
            size = size << 1;
    }
    return size;
}
//extension can be .c , .txt, .sh so after decoding size we decode those no .of bytes
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    decInfo->extn_size = decode_lsb_to_int(decInfo);
    //printf("size %d",decInfo->extn_size);
    return e_success;
}
//we decode extension and take file name without extension 
Status decode_secret_extn(DecodeInfo *decInfo)
{
    //decode extn and store in extention 
    char extention[decInfo->extn_size + 1];
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        //function call to decode
        extention[i] = decode_lsb_to_char(decInfo);
    }
    extention[decInfo->extn_size] = '\0';
    //copy till "." in given output file name
    char *temp = strstr(decInfo->decode_fname, ".");
    if (temp != NULL)
    {
        int i = 0;
        char str[100];
        while (decInfo->decode_fname[i] != '.')
        {
            str[i] = decInfo->decode_fname[i];
            i++;
        }
        str[i] = '\0';
        strcpy(decInfo->decode_fname, str);
    }
    //concate file name along with extension 
    strcat(decInfo->decode_fname, extention);
    printf("INFO: Creating %s \n", decInfo->decode_fname);

    decInfo->fptr_decode = fopen(decInfo->decode_fname, "w");
    //validate the file created
    if (decInfo->fptr_decode == NULL)
    {
        printf("ERROR: Unable to open output file\n");
        return e_failure;
    }
    printf("INFO: Opened %s\n",decInfo->decode_fname);
    return e_success;
}

//to know how many bytes to decode to get our secret data -->file size
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    //call function to decode 
    decInfo->size_secretfile = decode_lsb_to_int(decInfo);
    return e_success;
}

//we know file size then decode data
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    for (int i = 0; i < decInfo->size_secretfile; i++)
    {
        //call function to decode 
        ch = decode_lsb_to_char(decInfo);
        //then write into output file
        fwrite(&ch, 1, 1, decInfo->fptr_decode);
    }
    return e_success;
}

