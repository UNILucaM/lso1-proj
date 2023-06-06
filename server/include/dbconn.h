#include <libpq-fe.h>

#define REGISTERSTATEMENT "INSERT IN Account VALUES($1, $2);"
#define LOGINSTATEMENT "SELECT * FROM Account WHERE username = $1 AND password = $2;"
#define GETPRODUCTCOCKTAILSTATEMENT "SELECT * FROM Product WHERE productTypeVar = 'Cocktail';"
#define GETPRODUCTFRULLATOSTATEMENT "SELECT * FROM Product WHERE productTypeVar = 'Frullato';"
#define GETPRODUCTSUGGESTEDSCORESTATEMENT "SELECT * FROM get_products_with_suggestion_score($1) WHERE suggestionScore != 0 ORDER BY suggestionscore DESC;"

#define PRODUCT_TYPE_INVALID -1
#define PRODUCT_TYPE_COCKTAIL 1
#define PRODUCT_TYPE_FRULLATO 2
#define PRODUCT_TYPE_SUGGESTED 3

#define PRODUCT_COLUMN_PID 0
#define PRODUCT_COLUMN_PRODUCTNAME 1
#define PRODUCT_COLUMN_PRICE 2
#define PRODUCT_COLUMN_PRODUCTTYPEVAR 3
#define PRODUCT_COLUMN_IMAGEPATH 4
#define PRODUCT_COLUMN_SUGGESTIONSCORE 5

#define PRODUCT_NUMBER_OF_COLUMNS 5

#define OUT_OF_MEMORY -1

PGconn *get_db_conn(char*, char*, char*, char**);
char *make_json_array_from_productqueryresult(PGresult*, char*, bool);
