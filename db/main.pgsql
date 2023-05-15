/*
Definizione estensioni.
hstore: necessaria per l'hashmap utilizzata dalla funzione get_products_ordered_by_suggested.
*/

CREATE EXTENSION hstore;

/*
 Enum per la definizione del tipo di prodotto.
Sono disponibili due tipi di prodotto: cocktail e frullati.
*/

CREATE TYPE productType AS ENUM('Frullato', 'Cocktail');

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
L'imagePath descrive, se presente, il path dove è locata l'immagine del prodotto sul server.
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

/*
Tabella Sale. Rappresenta gli acquisti di un prodotto specifico da parte di un utente.
NOTA: SE SI AGGIORNA IL VALORE DI DEFAULT DELLA COLONNA lastTimeBought, DEVE ESSERE AGGIORNATA
ANCHE LA FUNZIONE update_product_stock_quantity_and_register_sale().
*/
CREATE TABLE Sale(
	username text,
	productPid integer,
	quantity integer NOT NULL,
	lastTimeBought timestamp NOT NULL DEFAULT (now()::timestamp),
	CONSTRAINT SALE_PK PRIMARY KEY(username, productPid),
	CONSTRAINT SALE_FK_ACCOUNT FOREIGN KEY (username) REFERENCES Account(username),
	CONSTRAINT SALE_FK_PRODUCT FOREIGN KEY (productPid) REFERENCES Product(pid),
	CONSTRAINT SALE_QUANTITY CHECK (quantity >= 0)
);
	
CREATE OR REPLACE procedure upsert_sale(pid integer, desiredQuantity integer, customerUsername text)
LANGUAGE plpsgsql
AS $$
BEGIN
	INSERT INTO Sale (username, productPid, quantity)
		VALUES (customerUsername, pid, desiredQuantity)
	ON CONFLICT (username, productPid) DO UPDATE
		SET quantity = sale.quantity + excluded.quantity;
END
$$

/*
Funzione trigger che riflette gli aggiornamenti/inserzioni sulla tabella Sale sulle quantità dei record delle tabelle Product relative.
Nel caso ci sia un aggiornamento, aggiorna anche l'ultima volta che il prodotto è stato comprato.
NOTA: SE SI AGGIORNA IL VALORE DI DEFAULT DELLA COLONNA lastTimeBought DELLA TABELLA Sale, 
DEVE ESSERE AGGIORNATA
ANCHE LA FUNZIONE update_product_stock_quantity_and_register_sale().
*/
CREATE OR REPLACE FUNCTION update_product_stock_quantity_and_register_sale()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
DECLARE
	quantityInStock integer := 0;
	desiredQuantity integer;
BEGIN
	IF TG_OP = 'INSERT' THEN
		desiredQuantity = NEW.quantity;
	ELSE
		desiredQuantity = NEW.quantity - OLD.quantity 
		IF desiredQuantity < 0 RETURN NEW;
	END IF;
	SELECT quantity INTO quantityInStock FOR UPDATE
	FROM Product AS P
	WHERE P.pid = productPid;
	IF (quantityInStock >= desiredQuantity) THEN
		UPDATE Product
		SET quantity = quantity - desiredQuantity
		WHERE P.pid = productPid;
		IF TG_OP = 'UPDATE' THEN
			NEW.lastTimeBought = CAST(now() AS timestamp);
		END IF;
	ELSE 
		RAISE EXCEPTION 'Cannot sell product: available quantity is less than the desired one.';
	END IF;
	RETURN NEW;
END
$$

CREATE OR REPLACE TRIGGER UPDATE_LAST_TIME_BOUGHT
BEFORE UPDATE ON Sale
FOR EACH ROW
EXECUTE PROCEDURE update_product_stock_quantity_and_register_sale();

/*
La base del nostro algoritmo per consigliare bevande agli utenti.
La funzione restituisce una table costituita da:
1. tutte le colonne contenute nella tabella prodotto
2. un punteggio secondo il quale viene valutato quanto "consigliabile" sia una bevanda,
calcolato controllando quante volte ogni ingrediente della bevanda appare nelle bevande acquistate da un utente:
il punteggio è costituito quindi dalla somma della somma di queste quantità e del numero di volte che l'utente ha acquistato
lo stesso tipo di bevanda.
*/

CREATE OR REPLACE FUNCTION get_products_with_suggestion_score(username text)
RETURNS table
(pid integer, productName text, price float, quantity integer, productTypeVar productType,
	imagePath text,  suggestionScore integer)
LANGUAGE plpgsql
AS $$
#variable_conflict use_variable
DECLARE
	hashmap hstore = '';
	typeHashmap hstore = '';
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
	productTypeCursor cursor(cusername text) for
		SELECT P.productTypeVar::text AS productTypeText, SUM(S.quantity) AS timesSold
		FROM Sale AS S JOIN Product as P ON S.productPid = P.pid
		WHERE S.username = cusername
		GROUP BY productTypeText;
	productTypeCursorRow record;
	tmpIngredientName text;
	tmpProductTypeText text;
	tmpSuggestionScore integer = 0;
BEGIN
	open ingredientCursor(username);
	LOOP
		fetch ingredientCursor into ingredientCursorRow;
		exit when not found;
		IF hashmap[ingredientCursorRow.ingredientName] IS NULL THEN
			hashmap[ingredientCursorRow.ingredientName] = 0;
		END IF;
		hashmap[ingredientCursorRow.ingredientName] = hashmap[ingredientCursorRow.ingredientName]::integer + ingredientCursorRow.timesSold;
	END LOOP;
	close ingredientCursor;
	open productTypeCursor(username);
	LOOP
		fetch productTypeCursor into productTypeCursorRow;
		exit when not found;
		hashmap[productTypeCursorRow.productTypeText] = productTypeCursorRow.timesSold;
	END LOOP;
	close productTypeCursor;
	FOR pid, productName, price, quantity, productTypeVar, imagePath, suggestionScore IN
		SELECT *, 0 as score
		FROM Product
	LOOP
		open secondIngredientCursor(pid);
		suggestionScore = 0;
		tmpSuggestionscore = 0;
		LOOP
			fetch secondIngredientCursor into tmpIngredientName;
			exit when not found;
			IF hashmap[tmpIngredientName] IS NULL THEN
				hashmap[tmpIngredientName] = 0;
			END IF;
			suggestionScore = suggestionScore + hashmap[tmpIngredientName]::integer;
		END LOOP;
		close secondIngredientCursor;
		SELECT P.productTypeVar::text INTO tmpProductTypeText FROM Product AS P WHERE P.pid = pid;
		IF hashmap[tmpProductTypeText] IS NULL THEN
			hashmap[tmpProductTypeText] = 0;
		END IF;
		suggestionScore = suggestionScore + hashmap[tmpProductTypeText]::integer;
		RETURN NEXT;
	END LOOP;
END
$$

/* 
Inserisce un prodotto con i suoi relativi ingredienti:
1. viene inserito il prodotto;
1. si controlla che esistano gli ingredienti;
	a. se non esistono, vengono inseriti;
3. Viene creato per ogni ingrediente un record nella tabella IngredientInProduct con relativo pid.
*/
CREATE OR REPLACE PROCEDURE insert_product_with_ingredients
	(productName text, price float, quantity integer, productTypeVar productType, imagePath text,
	VARIADIC ingredients text[])
LANGUAGE plpgsql
AS $$
DECLARE
	tpid integer;
	ingredient text;
BEGIN
	EXECUTE 
		'INSERT INTO Product VALUES(DEFAULT, $1, $2, $3, $4, $5) RETURNING pid;'
	INTO tpid
	USING productName, price, quantity, productTypeVar, imagePath;
	FOREACH ingredient IN ARRAY ingredients
	LOOP
		EXECUTE 
			'INSERT INTO Ingredient VALUES($1) ON CONFLICT DO NOTHING;
			INSERT INTO IngredientInProduct VALUES($1, $2);'
		USING ingredient, tpid;
	END LOOP;
END
$$