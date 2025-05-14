#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PAINTING_WIDTH 106
#define PAINTING_HEIGHT 70

int main(int argc, char **argv){
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_image> <output_image> <frame_number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int image_width, image_height, image_channels;
    int frame_width, frame_height, frame_channels;


    //Loads the input image
    unsigned char *input_data = stbi_load(argv[1], &image_width, &image_height, &image_channels, 4);
    if (!input_data) {
        fprintf(stderr, "Failed to load input image: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    image_channels = 4;


    //Loads the frame image
    char frame_file_name[100];
    snprintf(frame_file_name, sizeof(frame_file_name), "frames_greenscreened/frame_%s.png", argv[3]);

    unsigned char *frame_data = stbi_load(frame_file_name, &frame_width, &frame_height, &frame_channels, 4);
    frame_channels = 4;
    if (!frame_data) {
        fprintf(stderr, "Failed to load frame image: %s\n", frame_file_name);
        stbi_image_free(input_data);
        return EXIT_FAILURE;
    }


    //Verify dimension
    if (image_width != frame_width || image_height != frame_height) {
        fprintf(stderr, "Frame and input image dimensions do not match.\n");
        stbi_image_free(input_data);
        stbi_image_free(frame_data);
        return EXIT_FAILURE;
    }


    //Allocating memory to store the loaded images (for 3d array)
    unsigned char (*image)[image_width][image_channels] = malloc(image_height * sizeof(*image));
    unsigned char (*frame)[frame_width][frame_channels] = malloc(frame_height * sizeof(*frame));
    unsigned char (*painting)[PAINTING_WIDTH][image_channels] = malloc(PAINTING_HEIGHT * sizeof(*painting));
    if (!image || !painting || !frame) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(image);
        free(painting);
        free(frame);
        stbi_image_free(input_data); 
        stbi_image_free(frame_data);
        return EXIT_FAILURE;
    }
    
    //Copy image over to a 3D array
    for (int y = 0; y < image_height; y++) {
        for (int x = 0; x < image_width; x++) {
            for (int c = 0; c < image_channels; c++) {
                image[y][x][c] = input_data[(y * image_width + x) * image_channels + c];
            }
        }
    }

    //Copy frame over to a 3D array
    for (int y = 0; y < frame_height; y++) {
        for (int x = 0; x < frame_width; x++) {
            for (int c = 0; c < frame_channels; c++) {
                frame[y][x][c] = frame_data[(y * frame_width + x) * frame_channels + c];
            }
        }
    }

    stbi_image_free(input_data);
    stbi_image_free(frame_data);


    //Applying the frame over image
    for (int y = 0; y < image_height; y++) {
        for (int x = 0; x < image_width; x++) {
            if((frame[y][x][0] == 0) && (frame[y][x][1] == 255) && (frame[y][x][2] == 0) && (frame[y][x][3] == 255)){
                continue; //Keep original pixel in the image
            }
            //Replace image pixel with frame pixel
            for (int c = 0; c < image_channels; c++) {
                image[y][x][c] = frame[y][x][c];
            }
        }
    }


    //Reformat image into terraria image
    int image_y = 0;
    int image_x = 0;
    for (int y = 0; y < PAINTING_HEIGHT; y++) {

        if(((y%18) == 16) || ((y%18) == 17)){
            //Add transparancy rows
            for (int x = 0; x < PAINTING_WIDTH; x++){
               painting[y][x][0] = 255;
               painting[y][x][1] = 255;
               painting[y][x][2] = 255;
               painting[y][x][3] = 0;
            }
            continue;
        }


        for (int x = 0; x < PAINTING_WIDTH; x++) {
            if(((x%18) == 16) || ((x%18) == 17)){
                //Add transparancy columns
                painting[y][x][0] = 255;
                painting[y][x][1] = 255;
                painting[y][x][2] = 255;
                painting[y][x][3] = 0;
                continue;
            }


            for (int c = 0; c < image_channels; c++) {
                painting[y][x][c] = image[image_y][image_x][c];
            }
            image_x++;

        }
        image_x = 0;
        image_y++;
    }

    //Writing to create the final image
    if (!stbi_write_png(argv[2], PAINTING_WIDTH, PAINTING_HEIGHT, image_channels, painting, PAINTING_WIDTH * image_channels)) {
        fprintf(stderr, "Failed to write output image.\n");
    }

    //Freeing memory
    free(image);
    free(painting);
    free(frame);
    return EXIT_SUCCESS;
}
