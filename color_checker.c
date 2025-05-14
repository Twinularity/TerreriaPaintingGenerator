#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char **argv){
    //0 is executable name
    //1 is output file
    //2+ are input png files
    int input_file_count = argc - 2;
    char **input_files = &argv[2];
    if (argc < 3) {
    fprintf(stderr, "Usage: %s <output_file> [<input_file.png> ...]\n", argv[0]);
    return EXIT_FAILURE;
    }

    int width;
    int height;
    int rgb = 4; // 0 => R; 1 => G; 2 => B 3 => transparency
    int contains_color;
    FILE *output_file = fopen(argv[1], "w");
    for (int i=0; i < input_file_count; i++){
        contains_color = 0;
        unsigned char *data = stbi_load(input_files[i], &width, &height, &rgb, 4);
        if (!data) {
            fprintf(stderr, "Failed to load image: %s.\n", input_files[i]);
            return EXIT_FAILURE;
        }

        unsigned char (*image)[width][rgb] = malloc(height * sizeof(*image));
        if (!image) {
            fprintf(stderr, "Memory allocation failed.\n");
            stbi_image_free(data);
            return EXIT_FAILURE;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int c = 0; c < rgb; c++) {
                    image[y][x][c] = data[(y * width + x) * rgb + c];
                }
                contains_color = contains_color || ((image[y][x][0] == 0) && (image[y][x][1] == 255) && (image[y][x][2] == 0) && (image[y][x][3] == 255));
            }
        }
        fprintf(output_file, "File: %s. Contains_color: %d. \n", input_files[i], contains_color);
        stbi_image_free(data);
        free(image);
    }
    fclose(output_file);
    return EXIT_SUCCESS;
}