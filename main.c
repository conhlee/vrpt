#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <Windows.h>
#include "global.h"

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        if (strcmp("-extract", argv[1]) == 0) {
            if (argv[2]) extract(argv[2]);
            else {
                printf("There was an error:\nNo path was provided!");
                exit(1);
            }
        } else printf(HELPSTR, VERSION);
    } else {
        printf(HELPSTR, VERSION);
    }

    return 0;
}

int panic(char error[]) {
    printf("\nThere was an unexpected error:\n%s\n", error);
    exit(1);
}

int extract(char path[]) {
    printf("Creating directory..");

    char fullPath[PATH_MAX];
    GetFullPathName(path, PATH_MAX, fullPath, 0);

    char outputDir[PATH_MAX];
    strcpy(outputDir, fullPath);
    strncat(outputDir, "_output", PATH_MAX);

    if (CreateDirectory(outputDir, 0) == -1)
        panic("Unable to create main output directory!");
    printf(" Done!\n");


    printf("\nLoading package file..");

    FILE *binf = NULL;

    int32_t tfc = 0;

    if ((binf = fopen(fullPath, "rb")) == NULL)
        panic("Unable to open package file!");
    else
        printf(" Done!\n\nReading file count..");


    fread(&tfc, 1, sizeof(tfc), binf);
    printf(" %u files found!\nReading index..", (int)tfc);

    int32_t fileEntries[(int)tfc];

    int32_t buffer = 0;
    for (int i = 0; i < tfc; i++) {
        fread(&buffer, 1, sizeof(buffer), binf);
        fileEntries[i] = buffer;
    }

    printf(" Done!\nExtracting files..\n\n");

    for (int i = 0; i < tfc; i++) {
        fseek(binf, fileEntries[i], SEEK_SET);


        int32_t nameLen = 0;
        while (fgetc(binf) != NULL)
            nameLen++;

        fseek(binf, fileEntries[i], SEEK_SET);
        char pathName[nameLen + 1];
        
        fread(pathName, 1, sizeof(pathName), binf);


        printf("Extracting '%s'..\n", pathName);

        char *token;
        char seperator[2] = "/";

        token = strtok(pathName, seperator);

        char pathToCreate[PATH_MAX];
        strcpy(pathToCreate, outputDir);

        while (token != NULL) {
            strcat(pathToCreate, "\\");
            strcat(pathToCreate, token);

            if (strrchr(token, '.') == NULL) {
                DWORD dirAttributes = GetFileAttributes(pathToCreate);
                if ((dirAttributes == INVALID_FILE_ATTRIBUTES && (dirAttributes & FILE_ATTRIBUTE_DIRECTORY)))
                    if (CreateDirectory(pathToCreate, 0) == -1) panic("Unable to create secondary output directory!");
            }

            token = strtok(NULL, seperator);
        }
        fseek(binf, fileEntries[i] + nameLen + (3 - (nameLen % 4)) + 1, SEEK_SET);
        
        int32_t fileSize;
        fread(&fileSize, 1, sizeof(fileSize), binf);

        FILE *outFile;
        if ((outFile = fopen(pathToCreate, "wb")) == NULL)
            panic("Unable to create output file!");

        char storedFile[fileSize];
        fread(&storedFile, 1, fileSize, binf);

        fwrite(storedFile, 1, fileSize, outFile);
        fclose(outFile);
    }

    fclose(binf);
}