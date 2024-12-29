/*
Name : N venkata prasad
date : 29/09/24
decription : testencode.c -- consists of main function and required function calls
*/
#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    //check if arguments are less than 3
    if (argc < 3)
    {
        printf("Error:- Insufficient Arguments\n");
        printf("Usage:- For Encoding -> ./a.out -e <.bmp file> <.txt file>\n\tFor Decoding -> ./a.out -d <.bmp file> <output file>\n");
    }
    else
    {
        int type = check_operation_type(argv);
        if (type == e_encode)
        {
            printf("Started Encoding\n");

            EncodeInfo encInfo;
            uint img_size;
            printf("INFO : Validating arguments\n");
            // function call to validate arguments
            if (read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                printf("INFO : DONE\n");

                // function call to open files
                if (open_files(&encInfo) == e_failure)
                {
                    printf("ERROR: %s function failed\n", "open_files");
                    return 1;
                }
                else
                {
                    printf("SUCCESS: %s function completed\n", "open_files");
                    //if files opened successfully start encoding 
                    if (do_encoding(&encInfo) == e_success)
                    {
                        printf("Encoding completed successfully\n");
                    }
                    else
                    {
                        printf("ERROR: Encoding failed\n");
                    }
                }
            }
            //when arguments are not in correct format
            else
            {
            printf("Error:- Invalid Arguments\n");
            printf("Usage:- For Encoding -> ./a.out -e <.bmp file> <.txt file>\n\tFor Decoding -> ./a.out -d <.bmp file> <output file>\n");
            }
        }
        //if it is decoding
        else if (type == e_decode)
        {
            DecodeInfo decInfo;
            // function call for validating arguments
            if (read_and_validate_decode_args(argv, &decInfo) == e_success)
            {
                // function call for decoding
                if (do_decoding(&decInfo) == e_success)
                {
                    printf("Completed decoding\n");
                }
                else
                {
                    printf("Failed to decode\n");
                }
            }
            else
            {
            printf("Error:- Invalid Arguments\n");
            printf("Usage:- For Encoding -> ./a.out -e <.bmp file> <.txt file>\n\tFor Decoding -> ./a.out -d <.bmp file> <output file>\n");
            }
        }
    else
    {
        printf("ERROR: Unsupported operation\n");
    }
    }
    return 0;
}
