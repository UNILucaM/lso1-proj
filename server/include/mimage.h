#include <stdbool.h>
typedef enum{
	SMALL,
	MEDIUM,
	LARGE
}imagesize;

extern char *imagesizestringconversiontable[];

#define MALLOC_ERROR -1
#define STAT_ERROR -2
#define BAD_ARGS_ERROR -3
#define FILE_ERROR -4

#define IMAGE_SIZE_SEPARATOR '_'

/*Lo spazio allocato per il valore di return deve essere liberato.
Il secondo argomento serve come output opzionale della dimensione del file,
e, in caso di errore, come output per il codice d'errore.*/
char *load_image_from_path(char*, int*);

/*Restituisce il path del file a seconda della dimensione.
es ("/images/martini.jpg" -> "/images/martini_large.jpg")
Lo spazio allocato per il valore di return deve essere liberato.*/
char *get_path_for_size(char*, imagesize);
