#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "mimage.h"

char *imagesizestringconversiontable[] = {"small", "medium", "large"};

const char *imageloaderrorstrings[] = {
	"Could not allocate memory.",
	"Could not obtain information from stat() call.",
	"Bad arguments passed to function.",
	"Error opening or processing function.",
	"File is too large to open for this implementation."
}
	
	

char *load_image_from_path(char* path, int *len, time_t *timeToCompareWith, bool isIfModifiedSinceMode){
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
	time_t lastModified = stat->st_mtime;
	if (timeToComparewith != NULL){
		double comparisonResult = difftime
			(lastModified, *timeToCompareWith);
		if ((isModifiedSinceMode && comparisonResult < 0) || 
			(!isModifiedSinceMode && comparisonResult > 0){
			*len = NO_NEED_TO_LOAD;
			free(fileProperties);
			return NULL;
		}
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
			if (len != NULL) *len = FILE_ERROR;
			output = NULL;
		}
		else if (len != NULL) *len = (int) readResult;	
	}
	free(fileProperties);
	fclose(imageFile);
	return output;
}

char *get_path_for_size(char *path, imagesize sizeType){                 
	char *imageSizeString = imagesizestringconversiontable[sizeType];
	if (strcmp(imageSizeString, imagesizestringconversiontable[MEDIUM]) == 0) 
			return path;                        
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

int convert_string_to_imagesize_enum(char *str){
	int i = 0;
	for (; i < ENUM_IMAGESIZE_N; i++) 
		if (strcmp(imagesizestringconversiontable[i], str) == 0) return i;
	return SIZE_UNDEFINED;
}

const char *get_image_load_error_string(int errorNum){
	if (errorNum == 0) return NULL;
	if (errorNum < 0) errorNum = errorNum * -1;
	return imageloaderrorstrings[errorNum-1];
}