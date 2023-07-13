INSERT INTO Account VALUES ('AccountTest', 'a2cdE!789');
INSERT INTO Account VALUES ('AccountTest2', 'L65GfDq9P-');
INSERT INTO Account VALUES ('AccountTest3', 'L65Gfaz!aw');


INSERT INTO Ingredient VALUES('Latte', 100000);
-- productName, price, productTypeVar, imagePath, defaultQuantityInStock, lista(ingredientName, ingredientQuantityNeededInRecipe)

CALL insert_product_with_ingredients('Negroni', 5, 'Cocktail', NULL, 3000, ('Gin', 30), ('Campari', 30), ('Vermouth Rosso', 30));
CALL insert_product_with_ingredients('Bronx', 5, 'Cocktail', NULL, 2000, ('Gin', 30), ('Vermouth Rosso', 15), ('Vermouth Extra Dry', 7.5));
CALL insert_product_with_ingredients('Manhattan', 5, 'Cocktail', NULL, 1000, ('Rye Whisky', 27.5), ('Vermouth Rosso', 15), ('Angostura', 1));
CALL insert_product_with_ingredients('Martini', 5, 'Cocktail', NULL, 1000, ('Gin London Dry', 60), ('Vermouth Extra Dry', 30), ('Orange Bitter', 1));
CALL insert_product_with_ingredients('Margarita', 5, 'Cocktail', NULL, 1000, ('Tequila', 7.5), ('Triple Sec', 27.5), ('Succo di Lime', 27.5));

CALL insert_product_with_ingredients('Frullato alla Fragola', 3, 'Frullato', NULL, 10000, ('Latte', 700), ('Fragola', 450));
CALL insert_product_with_ingredients('Frullato alla Menta', 3.50, 'Frullato', NULL, 10000, ('Latte', 700), ('Sciroppo alla Menta', 200));
CALL insert_product_with_ingredients('Frullato al Limone', 3.50, 'Frullato', NULL, 10000, ('Latte', 700), ('Sciroppo al Limone', 105));
CALL insert_product_with_ingredients('Frullato All''Albicocca', 3, 'Frullato', NULL, 10000, ('Latte', 700), ('Albicocca', 350));

-- username, productPid, quantity, lastTimeBought
INSERT INTO Sale VALUES ('AccountTest', 1, 1, DEFAULT);
INSERT INTO Sale VALUES ('AccountTest', 3, 4, DEFAULT);
INSERT INTO Sale VALUES ('AccountTest', 6, 2, DEFAULT);
INSERT INTO Sale VALUES ('AccountTest2', 8, 5, DEFAULT);
INSERT INTO Sale VALUES ('AccountTest2', 2, 9, DEFAULT);
INSERT INTO Sale VALUES ('AccountTest2', 1, 2, DEFAULT);
INSERT INTO Sale VALUES ('AccountTest2', 3, 4, DEFAULT);

CALL upsert_sale('AccountTest2', 8, 3);
