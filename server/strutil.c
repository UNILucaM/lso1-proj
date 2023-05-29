//Implementazione migliore di strcat quando si 
//devono fare più concatenazioni di fila.
//Funziona nella seguente maniera,
//sfruttando che la condizione del while è vera
//finché la variabile è diversa da 0 (NULL)
//Quindi alla prima linea troviamo il puntatore a NULL,
//e successivamente concateniamo la stringa 
//(finché non giungiamo al NULL).
//Si nota che si ritorna "--dest" perché 
//oltrepassiamo il NULL all'ultimo incremento.
char *chainstrcat(char *dest, char *src){
	while (*dest) dest++;
    while (*dest++ = *src++);
    return --dest;
}