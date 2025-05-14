#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PAINTING_WIDTH 106
#define PAINTING_HEIGHT 70

unsigned char *load_image_rgba(const char *file_name, int *width, int *height); //Returns 0 if failure
void copy_to_3d_array(int width, int height, const unsigned char *flat, unsigned char (*array)[width][4]);
void apply_frame(int width, int height, unsigned char (*image)[width][4], const unsigned char (*frame)[width][4]);
void format_painting(int image_width, const unsigned char (*src)[image_width][4], unsigned char (*dest)[PAINTING_WIDTH][4]);
void cleanup_exit(void *input_data, void *frame_data, void *image, void *frame, void *painting, int exit_code);

int main(int argc, char **argv){
    unsigned char *input_data = NULL;
    unsigned char *frame_data = NULL;

    //Checking command line arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_image> <output_image> <frame_number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int image_width, image_height;
    int frame_width, frame_height;


    //Loads the input image
    input_data = load_image_rgba(argv[1], &image_width, &image_height);
    if (!input_data) cleanup_exit(input_data, frame_data, NULL, NULL, NULL, 1);

    //verify image dimensions
    if (image_width != 96 || image_height != 64) {
        fprintf(stderr, "Input image must be 96x64 pixels for proper formatting.\n");
        cleanup_exit(input_data, frame_data, NULL, NULL, NULL, 1);
    }

    //Allocating memory to store the loaded images (for 3d array)
    unsigned char (*image)[image_width][4] = malloc(image_height * sizeof(*image));
    unsigned char (*painting)[PAINTING_WIDTH][4] = malloc(PAINTING_HEIGHT * sizeof(*painting));
    if (!image || !painting) {
        fprintf(stderr, "Memory allocation failed.\n");
        cleanup_exit(input_data, frame_data, image, NULL, painting, 1);
    }
    copy_to_3d_array(image_width, image_height, input_data, image);
    stbi_image_free(input_data); 
    input_data = NULL;

    //Load and apply frame if a frame was choosen
    unsigned char (*frame)[image_width][4] = NULL; //We assume the image and frame widths are the same
    if(strcmp(argv[3], "0") != 0){
        //Loads the frame image
        char frame_file_name[100];
        snprintf(frame_file_name, sizeof(frame_file_name), "frames_greenscreened/frame_%s.png", argv[3]);
        frame_data = load_image_rgba(frame_file_name, &frame_width, &frame_height);
        if (!frame_data) cleanup_exit(input_data, frame_data, NULL, NULL, NULL, 1);

        //Verify dimensions
        if (image_width != frame_width || image_height != frame_height) {
            fprintf(stderr, "Frame and input image dimensions do not match.\n");
            cleanup_exit(input_data, frame_data, NULL, NULL, NULL, 1);
        }

        //Allocating memory to store the frame
        frame = malloc(frame_height * sizeof(*frame));
        if (!frame) {
            fprintf(stderr, "Memory allocation failed.\n");
            cleanup_exit(input_data, frame_data, image, frame, painting, 1);
        }
        copy_to_3d_array(frame_width, frame_height, frame_data, frame);
        stbi_image_free(frame_data); 
        frame_data = NULL;

        //Applying the frame over image
        apply_frame(image_width, image_height, image, frame);
    }

    //Reformat image into terraria image
    format_painting(image_width, image, painting);

    //Writing to create the final image
    if (!stbi_write_png(argv[2], PAINTING_WIDTH, PAINTING_HEIGHT, 4, painting, PAINTING_WIDTH * 4)) {
        fprintf(stderr, "Failed to write output image.\n");
        cleanup_exit(input_data, frame_data, image, frame, painting, 1);
    }

    //Freeing memory
    cleanup_exit(input_data, frame_data, image, frame, painting, 0);
    return EXIT_SUCCESS;
}


//This function loads input images into RGB
unsigned char *load_image_rgba(const char *file_name, int *width, int *height){
    int channels;
    unsigned char *data = stbi_load(file_name, width, height, &channels, 4);
    if(!data){
        fprintf(stderr, "Failed to load image: %s\n", file_name);
        return NULL; 
    }
    return data;
}

//This function takes the 1D array returned by stbi_load and converts it into a 3D format (is assumed that input is RGBA)
void copy_to_3d_array(int width, int height, const unsigned char *flat, unsigned char (*array)[width][4]){
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < 4; c++) {
                array[y][x][c] = flat[(y * width + x) * 4 + c];
            }
        }
    }
}

//This function takes the 3d frame and image arrays, and applies the frame on top of the image (RGBA assumed; Same image dimensions assumed)
void apply_frame(int width, int height, unsigned char (*image)[width][4], const unsigned char (*frame)[width][4]){
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if((frame[y][x][0] == 0) && (frame[y][x][1] == 255) && (frame[y][x][2] == 0) && (frame[y][x][3] == 255)){ 
                continue; //Keep original pixel in the image
            }
            //Replace image pixel with frame pixel
            for (int c = 0; c < 4; c++) {
                image[y][x][c] = frame[y][x][c];
            }
        }
    }
}

//This function reformats the image into a terreria readable format
void format_painting(int image_width, const unsigned char (*src)[image_width][4], unsigned char (*dest)[PAINTING_WIDTH][4]){
    int src_y = 0;
    for (int y = 0; y < PAINTING_HEIGHT; y++) {

        if(((y%18) == 16) || ((y%18) == 17)){
            //Add transparancy rows
            for (int x = 0; x < PAINTING_WIDTH; x++){
               dest[y][x][0] = 255;
               dest[y][x][1] = 255;
               dest[y][x][2] = 255;
               dest[y][x][3] = 0;
            }
            continue;
        }

        int src_x = 0;
        for (int x = 0; x < PAINTING_WIDTH; x++) {
            if(((x%18) == 16) || ((x%18) == 17)){
                //Add transparancy columns
                dest[y][x][0] = 255;
                dest[y][x][1] = 255;
                dest[y][x][2] = 255;
                dest[y][x][3] = 0;
                continue;
            }


            for (int c = 0; c < 4; c++) {
                dest[y][x][c] = src[src_y][src_x][c];
            }
            src_x++;

        }
        src_y++;
    }
}

void cleanup_exit(void *input_data, void *frame_data, void *image, void *frame, void *painting, int exit_code){
    free(input_data);
    free(frame_data);
    free(image);
    free(frame);
    free(painting);
    exit(exit_code);
}