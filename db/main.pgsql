/*
 Enum per la definizione del tipo di prodotto.
Sono disponibili due tipi di prodotto: cocktail e frullati.
*/

CREATE TYPE productType AS ENUM('Frullato, Cocktail');

/*
 Tabella user. 
Il nome utente deve essere tra 4 e 16 caratteri.
Il nome utente deve includere solo caratteri alfanumerici.
Il nome utente non inizia con un carattere numerico come suo primo carattere.
*/
CREATE TABLE Account(
	username text,
	passwordHash text NOT NULL,
	passwordSalt text NOT NULL,
	CONSTRAINT ACCOUNT_PK PRIMARY KEY(username),
	CONSTRAINT ACCOUNT_NAME_LENGTH CHECK(length(username) <= 16 AND
		length(username) > 4),
	CONSTRAINT ACCOUNT_NAME_FORMAT CHECK(username ~ '^(?![0-9])[a-zA-Z0-9]+$'),
	CONSTRAINT ACCOUNT_PASSWORDHASH_LENGTH CHECK(length(passwordHash) = 64)
);

/*
Tabella prodotto.
Per il prezzo sono permesse solo due cifre decimali.
*/
CREATE TABLE Product(
	pid serial,
	productName text NOT NULL UNIQUE,
	price float NOT NULL,
	quantity integer NOT NULL,
	productTypeVar productType NOT NULL,
	imagePath text,
	CONSTRAINT PRODUCT_PK PRIMARY KEY(pid),
	CONSTRAINT PRODUCT_QUANTITY CHECK (quantity >= 0),
	CONSTRAINT PRODUCT_PRICE_FORCE_TWO_DECIMAL_DIGITS CHECK (price = round(price::numeric, 2))
);

CREATE TABLE Ingredient(
	ingredientName text,
	CONSTRAINT INGREDIENT_PK PRIMARY KEY(ingredientName)
);

CREATE TABLE IngredientInProduct(
	ingredientName text,
	productPid integer,
	CONSTRAINT INGREDIENTINPRODUCT_PK PRIMARY KEY(ingredientName, productPid),
	CONSTRAINT INGREDIENTINPRODUCT_FK_INGREDIENT FOREIGN KEY (ingredientName) REFERENCES Ingredient(ingredientName),
	CONSTRAINT INGREDIENTINPRODUCT_FK_PRODUCT FOREIGN KEY (productPid) REFERENCES Product(pid)
);	 	

CREATE TABLE Sale(
	username text,
	productPid integer,
	quantity integer NOT NULL,
	lastTimeBougth timestamp NOT NULL,
	CONSTRAINT SALE_PK PRIMARY KEY(username, productPid),
	CONSTRAINT SALE_FK_ACCOUNT FOREIGN KEY (username) REFERENCES Account(username),
	CONSTRAINT SALE_FK_PRODUCT FOREIGN KEY (productPid) REFERENCES Product(pid),
	CONSTRAINT SALE_QUANTITY CHECK (quantity >= 0)
);

/*
La base del nostro algoritmo per consigliare bevande agli utenti.
La funzione restituisce una table costituita da:
1. tutte le colonne contenute nella tabella prodotto
2. un punteggio secondo il quale viene valutato quanto "consigliabile" sia una bevanda,
calcolato controllando quante volte ogni ingrediente della bevanda appare nelle bevande acquistate da un utente:
il punteggio è costituito quindi dalla somma della somma di queste quantità e del numero di volte che l'utente ha acquistato
lo stesso tipo di bevanda.
*/

CREATE OR REPLACE FUNCTION get_products_ordered_by_suggested(username text)
RETURNS table
(pid integer, productName text, price float, quantity integer, productTypeVar productType,
	imagePath text,  suggestionScore integer)
LANGUAGE plpgsql
AS $$
DECLARE
	hashmap hstore = '';
	ingredientCursor cursor(cusername text) for
		SELECT IIP.ingredientName, SUM(S.quantity) AS timesSold
		FROM Sale AS S JOIN IngredientInProduct AS IIP ON S.productPid = IIP.productPid
		WHERE S.username = cusername
		GROUP BY IIP.ingredientName;
	ingredientCursorRow record;
	secondIngredientCursor cursor(cpid integer) for
		SELECT IIP.ingredientName
		FROM IngredientInProduct AS IIP
		WHERE IIP.productPid = cpid;
	tmpIngredientName text;
	tmpSuggestionScore integer = 0;
BEGIN
	open ingredientCursor(username);
	LOOP
		fetch ingredientCursor into ingredientCursorRow;
		exit when not found;
		hashmap[ingredientCursorRow.ingredientName] = hashmap[ingredientCursorRow.ingredientName]::integer + ingredientCursorRow.timesSold;
	END LOOP;
	close ingredientCursor;
	FOR pid, productName, price, quantity, productTypeVar, imagePath, suggestionScore IN
		SELECT *, 0 as score
		FROM Product
	LOOP
		open secondIngredientCursor(pid);
		tmpSuggestionscore = 0;
		LOOP
			fetch secondIngredientCursor into tmpIngredientName;
			exit when not found;
			suggestionScore = suggestionScore + hashmap[secondIngredientCursor.ingredientName]::integer;
		END LOOP;
		close secondIngredientCursor;
		RETURN NEXT;
	END LOOP;
END
$$
