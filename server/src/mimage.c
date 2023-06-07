#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "mimage.h"

char *load_image_from_path(char* path, int *len){
	if (path == NULL){
		if (len != NULL) *len = BAD_ARGS_ERROR;
	}
	stat *fileProperties = malloc(sizeof(stat));
	if (fileProperties == NULL){
		if (len != NULL) *len = MALLOC_ERROR;
	}
	if (stat(path, fileProperties) != 0){
		free(fileProperties);
		if (len != NULL) *len = STAT_ERROR;
		return NULL;
	}
	off_t imageSize = stat->st_size;
	if (imageSize > INT_MAX){
		free(fileProperties);
		if (len != NULL) *len = FILE_TOO_LARGE_ERROR;
		return NULL;
	}
	char *output = malloc(sizeof(char)*imageSize);
	if (output == NULL){
		free(fileProperties);
		if (len != NULL) *len = MALLOC_ERROR;
		return NULL;
	}
	FILE *imageFile = fopen(path, "r");
	if (imageFile == NULL){
		free(output);
		free(fileProperties);
		if (len != NULL) *len = FILE_ERROR;
		return NULL;
	}
	size_t readResult;
	if ((readResult = fread(output, 1, imageSize, imageFile)) < imageSize){
		if (ferror(imageFile) != 0){
			free(output);
			output = NULL;
		}
		else if (len != NULL) *len = (int) readResult;	
	}
	free(fileProperties);
	fclose(imageFile);
	return output;
}

char *imagesizestringconversiontable[] = {"small", NULL, "large"};

char *get_path_for_size(char *path, imagesize sizeType){                 
        char *imageSizeString = imagesizestringconversiontable[sizeType];
        if (imageSizeString == NULL) return path;                        
        char *newPathStart = malloc(sizeof(char)*(strlen(imageSizeString)     
                + strlen(path) + 2));                                    
        if (newPathStart == NULL) return NULL;  
        char *newPath = newPathStart;
        while (*path != '.' && *path != '\0'){                           
                *newPath++ = *path++;                                    
        }
        if (*path == '\0'){                                              
                free(newPath);                                           
                return NULL;                                             
        }                                                                
        *newPath++ = IMAGE_SIZE_SEPARATOR;                               
        while (*imageSizeString != '\0'){                                         
                *newPath++ = *imageSizeString++;                         
        }
        while (*path != '\0'){
            *newPath++ = *path++;
        }            
        return newPathStart;                                                  
}
