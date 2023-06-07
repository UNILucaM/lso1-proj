#include <stdbool.h>
#include <sys/types.h>
typedef enum{
	SIZE_UNDEFINED = -1,
	SMALL,
	MEDIUM,
	LARGE
}imagesize;

//Numero di valori dell'enum validi.
#define ENUM_IMAGESIZE_N 3
#define IMAGES_PATH "images/"

extern char *imagesizestringconversiontable[];
extern const char *imageloaderrorstrings[];

#define NO_NEED_TO_LOAD 0
#define MALLOC_ERROR -1
#define STAT_ERROR -2
#define BAD_ARGS_ERROR -3
#define FILE_ERROR -4
#define FILE_TOO_LARGE_ERROR -5

#define IMAGE_SIZE_SEPARATOR '_'

/*Lo spazio allocato per il valore di return deve essere liberato.
Il secondo argomento serve come output opzionale della dimensione del file,
e, in caso di errore, come output per il codice d'errore.*
Il terzo argomento, se presente (non NULL), abilita il confronto in base al tempo di modifica.
L'ultimo argomento indica la modalità di confronto da usare:
se è falso, l'immagine viene caricata solo se non è stata modificata dall'ultima data.
Altrimenti, viene caricata se è stata modificata dall'ultima data.*/
char *load_image_from_path_timebased(char*, int*, time_t*, bool);
/*Restituisce il path del file a seconda della dimensione.
es ("/images/martini.jpg" -> "/images/martini_large.jpg")
Lo spazio allocato per il valore di return deve essere liberato.*/
char *get_path_for_size(char*, imagesize);
int convert_string_to_imagesize_enum(char*);
const char *get_image_load_error_string(int);
