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
	password text NOT NULL,
	CONSTRAINT ACCOUNT_PK PRIMARY KEY(username),
	CONSTRAINT ACCOUNT_NAME_LENGTH CHECK(length(username) <= 16 AND
		length(username) > 4),
	CONSTRAINT ACCOUNT_NAME_FORMAT CHECK(username ~ '^(?![0-9])[a-zA-Z0-9]+$'),
	CONSTRAINT ACCOUNT_PASSWORD_FORMAT CHECK(password ~ '(?=.{9,})(?=.*?[^\w\s])(?=.*?[0-9])(?=.*?[A-Z]).*?[a-z].*')
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
	productTypeVar productType NOT NULL,
	imagePath text,
	ingredientList text NOT NULL DEFAULT '',
	CONSTRAINT PRODUCT_PK PRIMARY KEY(pid),
	CONSTRAINT PRODUCT_PRICE_FORCE_TWO_DECIMAL_DIGITS CHECK (price = round(price::numeric, 2))
);

/*
La quantità è in ml.
*/

CREATE TABLE Ingredient(
	ingredientName text,
	quantity float,
	CONSTRAINT INGREDIENT_PK PRIMARY KEY(ingredientName),
	CONSTRAINT INGREDIENT_QUANTITY CHECK (quantity >= 0)
);

CREATE TABLE IngredientInProduct(
	ingredientName text,
	productPid integer,
	quantityNeededInRecipe float,
	CONSTRAINT INGREDIENTINPRODUCT_PK PRIMARY KEY(ingredientName, productPid),
	CONSTRAINT INGREDIENTINPRODUCT_FK_INGREDIENT FOREIGN KEY (ingredientName) REFERENCES Ingredient(ingredientName),
	CONSTRAINT INGREDIENTINPRODUCT_FK_PRODUCT FOREIGN KEY (productPid) REFERENCES Product(pid),
	CONSTRAINT INGREDIENTINPRODUCT_QUANTITY CHECK (quantityNeededInRecipe >= 0)
);

CREATE OR REPLACE FUNCTION update_product_ingredientList()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
DECLARE
	newIngredientList text;
BEGIN
	SELECT ingredientList INTO newIngredientList 
	FROM Product FOR NO KEY UPDATE
	WHERE pid = NEW.productPid;
	IF (newIngredientList = '') THEN
		newIngredientList = NEW.productName;
	ELSE
		newIngredientList = newIngredientList || ', ' || NEW.productName;
	END IF;
	UPDATE Product 
	SET ingredientList = newIngredientList
	WHERE pid = NEW.productPid;
	RETURN NEW;
END
$$;

CREATE TRIGGER ADD_INGREDIENTINPRODUCT_TO_INGREDIENT_LIST
AFTER INSERT ON IngredientInProduct
FOR EACH ROW
EXECUTE PROCEDURE update_product_ingredientList();

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
	
CREATE OR REPLACE procedure upsert_sale(customerUsername text, pid integer, desiredQuantity integer)
LANGUAGE plpgsql
AS $$
BEGIN
	INSERT INTO Sale (username, productPid, quantity)
		VALUES (customerUsername, pid, desiredQuantity)
	ON CONFLICT (username, productPid) DO UPDATE
		SET quantity = sale.quantity + excluded.quantity;
END
$$;

CREATE OR REPLACE FUNCTION get_how_many_products_can_be_made(productPid integer)
RETURNS integer
LANGUAGE plpgsql
AS $$
DECLARE
        minimumNumberOfPossibleUsages integer;
        isFirstIteration boolean := TRUE;
        ingredientCursor cursor(cpid integer) for
        	SELECT I.ingredientName, (floor((I.quantity::numeric/ IIP.quantityNeededInRecipe))) AS numberOfPossibleUsages
        	FROM Ingredient as I JOIN IngredientInProduct AS IIP ON I.ingredientName = IIP.ingredientName
        	WHERE IIP.productPid = cpid	
        	FOR NO KEY UPDATE;
        ingredientCursorRow record;
BEGIN
        open ingredientCursor(productPid);
        LOOP
        	fetch ingredientCursor into ingredientCursorRow;
        	exit when not found;
        	IF (ingredientCursorRow.numberOfPossibleUsages <= 0) THEN 
        		RETURN 0; 
        	END IF;
        	IF (isFirstIteration = TRUE) THEN
        		isFirstIteration = FALSE;
        		minimumNumberOfPossibleUsages = ingredientCursorRow.numberOfPossibleUsages;
        	ELSIF (minimumNumberOfPossibleUsages > ingredientCursorRow.numberOfPossibleUsages) THEN
        		minimumNumberOfPossibleUsages = ingredientCursorRow.numberOfPossibleUsages;
        	END IF;
        END LOOP;
        close ingredientCursor;
        RETURN minimumNumberOfPossibleUsages;
END
$$;

CREATE OR REPLACE PROCEDURE update_ingredients_quantity(productPid integer, cquantity integer)
LANGUAGE plpgsql
AS $$
DECLARE
	ingredientCursor cursor(cpid integer) for 
		SELECT IIP.ingredientName, IIP.quantityNeededInRecipe
		FROM IngredientInProduct AS IIP
		WHERE IIP.productPid = cpid;
	ingredientCursorRow record;
BEGIN
	open ingredientCursor(productPid);
	LOOP
		fetch ingredientCursor into ingredientCursorRow;
		exit when not found;
		UPDATE Ingredient AS I
		SET quantity = I.quantity - (cquantity * ingredientCursorRow.quantityNeededInRecipe)
		WHERE I.ingredientName = ingredientCursorRow.ingredientName;
	END LOOP;
	close ingredientCursor;
END
$$;

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
		desiredQuantity = NEW.quantity - OLD.quantity;
		IF desiredQuantity <= 0 THEN 
			RETURN NEW;
		END IF;
	END IF;
	quantityInStock = get_how_many_products_can_be_made(NEW.productPid);
	IF (quantityInStock >= desiredQuantity) THEN
		CALL update_ingredients_quantity(NEW.productPid, desiredQuantity);
		IF TG_OP = 'UPDATE' THEN
			NEW.lastTimeBought = CAST(now() AS timestamp);
		END IF;
	ELSE 
		RAISE EXCEPTION 'Cannot sell product: available quantity is less than the desired one.';
	END IF;
	RETURN NEW;
END
$$;

CREATE OR REPLACE TRIGGER UPDATE_INGREDIENT_QUANTITIES_AND_REGISTER_SALE
BEFORE UPDATE OR INSERT ON Sale
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
(pid integer, productName text, price float, productTypeVar productType,
	imagePath text, ingredientList text, suggestionScore integer)
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
	FOR pid, productName, price, productTypeVar, imagePath, ingredientList, suggestionScore IN
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
$$;

CREATE TYPE ingredientWithQuantityType AS (ingredientName text, quantity float);

/* 
Inserisce un prodotto con i suoi relativi ingredienti:
1. viene inserito il prodotto;
1. si controlla, per ogni ingredientWithQuantity in ingredientsWithQuantity, l'esistenza dell'ingrediente specificato con ingredientName:
	a. se non esiste, viene inserito in Ingredient con quantityInStock settato a defaultNewIngredientQuantityInStock;
3. Viene creato per ogni ingrediente un record nella tabella IngredientInProduct con relativo pid e quantity (specificato in ingredientWithQuantityType).
*/
CREATE OR REPLACE PROCEDURE insert_product_with_ingredients
	(productName text, price float, productTypeVar productType, imagePath text,
	defaultNewIngredientQuantityInStock float,
	VARIADIC ingredientsWithQuantity ingredientWithQuantityType[])
LANGUAGE plpgsql
AS $$
DECLARE
	tpid integer;
	ingredientWithQuantity ingredientWithQuantityType;
BEGIN
	EXECUTE 
		'INSERT INTO Product VALUES(DEFAULT, $1, $2, $3, $4) RETURNING pid;'
	INTO tpid
	USING productName, price, productTypeVar, imagePath;
	FOREACH ingredientWithQuantity IN ARRAY ingredientsWithQuantity
	LOOP
		EXECUTE 
			'INSERT INTO Ingredient VALUES($1, $4) ON CONFLICT DO NOTHING;
			INSERT INTO IngredientInProduct VALUES($1, $2, $3);'
		USING ingredientWithQuantity.ingredientName, tpid, ingredientWithQuantity.quantity, defaultNewIngredientQuantityInStock;
	END LOOP;
END
$$;
