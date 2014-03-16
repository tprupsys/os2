﻿// -----------------------------------------------------------------------
//
//
//	Tomas Petras Rupšys ir Dominykas Šiožinis
//	Operacinių Sistemų II-ojo atsiskaitymo realizacija
//
//
// -----------------------------------------------------------------------
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// C boolean apibrėžimas
#define TRUE 1
#define FALSE 0

// Darbo pabaigos tekstas
#define END_OF_WORK "\n\nDarbas baigtas..."

// Klaidų tekstai
#define FILE_FORMAT_ERROR_LENGTH "Klaida! Failo ilgis neatitinka reikalavimu!"
#define FILE_FORMAT_ERROR_BEGINNING "Klaida! Failo pradzios zyme - neteisinga!"
#define FILE_FORMAT_ERROR_ENDING "Klaida! Failo pabaigos zyme - neteisinga!"
#define FILE_FORMAT_ERROR_INCORRECT_LINES_LENGTH "Klaida! Programos maksimalus spausdinamų eiluciu kiekis - neteisingas!"
#define FILE_FORMAT_ERROR_INCORRECT_NAME "Klaida! Programos pavadinimas nenurodytas!"
#define FILE_FORMAT_WORD_LENGTH_INCORRECT "Klaida! Programoje yra netinkamas kiekis simboliu!"
#define INCORRECT_PARAMETERS "Klaida! Neteisingi parametrai!"
#define FILE_DOES_NOT_EXIST "Klaida! Failas neegzistuoja!"
#define MEMORY_ERROR_DATA_FAILURE "Klaida! Nepavyko irasyti i atminti duotuju duomenu"
#define MEMORY_ERROR_CODE_FAILURE "Klaida! Nepavyko irasyti kodo eiluciu i atminti!"

// Atminties spausdinimo tekstai
#define MEMORY_STATUS_TEXT_FIRST_LINE "Atminties bukle:"
#define MEMORY_STATUS_TEXT_WORD_LINE "-as zodis"

// Programos failų formato reikalavimai
#define FILE_FORMAT_BEGINNING "$BEG"
#define FILE_FORMAT_ENDING "$END"
#define FILE_FORMAT_MAX_LENGTH 444
#define FILE_FORMAT_PRINT_LINES_LENGTH 4
#define FILE_FORMAT_PROGRAM_MAX_LINES_FROM 4
#define FILE_FORMAT_PROGRAM_MAX_LINES_TO 7
#define FILE_FORMAT_PROGRAM_NAME_LENGTH 32
#define FILE_FORMAT_PROGRAM_NAME_END_SYMBOL '$'
#define FILE_FORMAT_PROGRAM_NAME_FROM 8
#define FILE_FORMAT_WORD_LENGTH 4

// OS Atminties specifikacija
#define OS_DESIGN_BLOCKS_FOR_VM 16
#define OS_DESIGN_WORDS_AMOUNT 512
#define OS_DESIGN_BLOCKS_AMOUNT 32
#define OS_DESIGN_WORDS_IN_BLOCK 16
#define OS_DESIGN_BYTES_PER_WORD 4
#define OS_DESIGN_EMPTY_WORD_BYTE_SYMBOL '*'
#define OS_DESIGN_RESERVED_WORD_SYMBOL '-'
#define OS_DESIGN_DATA_BLOCK_FROM 0
#define OS_DESIGN_DATA_BLOCK_TO 6
#define OS_DESIGN_CODE_BLOCK_FROM 7
#define OS_DESIGN_CODE_BLOCK_TO 13
#define OS_DESIGN_STACK_BLOCK_FROM 14
#define OS_DESGIN_STACK_BLOCK_TO 15
#define OS_DESIGN_DEFAULT_SP_VALUE 223
#define OS_DESIGN_DEFAULT_PC_VALUE 112

// Atmintis
char memory[OS_DESIGN_WORDS_AMOUNT][OS_DESIGN_BYTES_PER_WORD];

// Registras/adresas, rodantis į steko viršūnės
int sp;

// Registras/adresas, rodantis sekančios instrukcijos reikšmę
int pc;

// Registras, rodantis į puslapių lentelę
char ptr[4];

/**
 * Užpildo atminties masyvą simboliais, reiškiančiais tuščią baitą.
 */
void initialize_memory() {  
  for (int i=0 ; i<OS_DESIGN_WORDS_AMOUNT ; i++) {
    for (int j=0 ; j<OS_DESIGN_BYTES_PER_WORD ; j++) {
	  memory[i][j] = OS_DESIGN_EMPTY_WORD_BYTE_SYMBOL;
	}
  }
}

/**
 * Išspausdina atminties būklę.
 */
void print_memory_status() {
  printf("\n%s\n", MEMORY_STATUS_TEXT_FIRST_LINE);
  for (int i=0 ; i<OS_DESIGN_WORDS_AMOUNT ; i++) {
	for (int j=0 ; j<OS_DESIGN_BYTES_PER_WORD ; j++) {
	  printf("%c", memory[i][j]);
	}
	printf(": %d%s\n", (i+1), MEMORY_STATUS_TEXT_WORD_LINE);
  }
}

/**
 * Išspausdina puslapiavimo lentelę
 */
void print_page_table() {
  int pageTableBlockIndex = atoi(ptr);
  int pageTableFirstWordIndex = pageTableBlockIndex * OS_DESIGN_WORDS_IN_BLOCK;

  printf("VM :: RM\n");
  for (int i=0 ; i<OS_DESIGN_BLOCKS_FOR_VM ; i++) {
    printf("%d :: %s\n", i, memory[pageTableFirstWordIndex + i]);
  }
}

/**
 * Paverčia skaičių kaip simbolį. Galioja tik rėžiuose [0 ; 9].
 */
char int_to_char_symbol(int number) {
  return (char)((int)'0' + number);
}

/**
 * Rezervuoja bloką. Techniškai, pakeičia kiekvieną žodžio baitą iš
 * OS_DESIGN_EMPTY_WORD_BYTE_SYMBOL į OS_DESIGN_RESERVED_WORD_SYMBOL.
 */
void reserve_block(int blockIndex) {
  int wordsFrom = blockIndex * OS_DESIGN_WORDS_IN_BLOCK;
  int wordsTo = blockIndex * OS_DESIGN_WORDS_IN_BLOCK + OS_DESIGN_WORDS_IN_BLOCK;
  
  for (int i=wordsFrom ; i<wordsTo ; i++) {
    for (int j=0 ; j<OS_DESIGN_BYTES_PER_WORD ; j++) {
	  memory[i][j] = OS_DESIGN_RESERVED_WORD_SYMBOL;
	}
  }
}

/**
 * Įdiegia PTR registrą su reikšme, kur saugoma puslapiavimo lentelė.
 */
void initialize_ptr() {
  // Įdiegiame atsitikinių skaičių generatorių pagal laiką nuo 1970.01.01
  srand(time(NULL));
  
  // Parenkame atsitiktinį bloką puslapiavimo lentelei
  int pageTableBlockIndex = rand() % OS_DESIGN_BLOCKS_AMOUNT;
  int a1 = 0;
  int a2 = 0;
  int a3 = pageTableBlockIndex / 10;
  int a4 = pageTableBlockIndex % 10;

  // Įrašome reikšmę į PTR
  ptr[0] = int_to_char_symbol(a1);
  ptr[1] = int_to_char_symbol(a2);
  ptr[2] = int_to_char_symbol(a3);
  ptr[3] = int_to_char_symbol(a4);
  
  // Rezervuojame bloką puslapiavimo lentelei
  reserve_block(pageTableBlockIndex);
}

/**
 * Tikrina, ar realios atminties blokas yra tuščias.
 */
int is_block_empty(int blockIndex) {
  int wordsFrom = blockIndex * OS_DESIGN_WORDS_IN_BLOCK;
  int wordsTo = blockIndex * OS_DESIGN_WORDS_IN_BLOCK + OS_DESIGN_WORDS_IN_BLOCK;
  
  for (int i=wordsFrom ; i<wordsTo ; i++) {
    for (int j=0 ; j<OS_DESIGN_BYTES_PER_WORD ; j++) {
	  if (memory[i][j] != OS_DESIGN_EMPTY_WORD_BYTE_SYMBOL) {
	    return FALSE;
	  }
	}
  }
  
  return TRUE;
}

/**
 * Įrašo bloko atitikmenį į puslapiavimo lentelę.
 */
void write_block_adress_to_page_table(int virtualIndex, int realIndex) {
  int pageTableBlockIndex = atoi(ptr);
  int pageTableFirstWordIndex = pageTableBlockIndex * OS_DESIGN_WORDS_IN_BLOCK;
  sprintf(memory[pageTableFirstWordIndex + virtualIndex], "%d", realIndex);
}

/**
 * Įdiegia puslapiavimo lentelę.
 */
void initialize_page_table() {
  // Įdiegiame atsitikinių skaičių generatorių pagal laiką nuo 1970.01.01
  srand(time(NULL));

  // Iteruojame per nuo 0 iki tiek blokų kiek reikia VM
  for (int i=0 ; i<OS_DESIGN_BLOCKS_FOR_VM ; i++) {
    int randomBlockIndex = -1;
	int isBlockEmpty = FALSE;
	
	// Generuojame atsitiktinį bloko numerį kol randame tuščią
	do {
	  randomBlockIndex = rand() % OS_DESIGN_BLOCKS_AMOUNT;
	  isBlockEmpty = is_block_empty(randomBlockIndex);
	} while (isBlockEmpty == FALSE);
	
	// Rezervuojame bloką
	reserve_block(randomBlockIndex);
	
	// Įrašome į puslapiavimo lentelę adresą
	write_block_adress_to_page_table(i, randomBlockIndex);
  }
}

/**
 * Gražina žodžio virtualų adresą.
 */
int get_real_word_address(int virtualWordAddress) {
  return (OS_DESIGN_BLOCKS_FOR_VM * atoi(memory[OS_DESIGN_BLOCKS_FOR_VM * atoi(ptr) + virtualWordAddress / OS_DESIGN_BLOCKS_FOR_VM])) + 
    (virtualWordAddress % OS_DESIGN_BLOCKS_FOR_VM);
}

/**
 * Įdiegia pradinį adresą, į kurį rodo SP.
 */
void initialize_sp() {
  sp = OS_DESIGN_DEFAULT_SP_VALUE;
}

/**
 * Įdiegia pradinį adresą, į kurį rodo PC.
 */
void initialize_pc() {
  pc = OS_DESIGN_DEFAULT_PC_VALUE;
}

/***
 * Tikrina, ar vartotojo pateikti programos argumentai yra teisingi.
 */
int is_argument_data_correct(int argc, const char* argv[]) {
  return argc == 2 ? TRUE : FALSE;
}

/**
 * Laukia vartotojo įvesties.
 */
void wait_for_user_interaction() {
  printf("%s\n", END_OF_WORK);
  std::getchar();
}

/**
 * Tikrina, ar failas atitinka reikalaujamą formatą.
 */
int is_file_of_required_format(char *fileByteArray) {
  // Tikriname masyvo ilgį
  if (sizeof(fileByteArray) > FILE_FORMAT_MAX_LENGTH) {
	printf("%s\n", FILE_FORMAT_ERROR_LENGTH);
	return FALSE;
  }
  
  // Tikriname pradžios žymę
  int actualFormatBeginningLength = strlen(FILE_FORMAT_BEGINNING);
  if (strncmp(fileByteArray, FILE_FORMAT_BEGINNING, actualFormatBeginningLength) != 0) {
    printf("%s\n", FILE_FORMAT_ERROR_BEGINNING);
	return FALSE;
  }
  
  // Tikriname pabaigos žymę
  int actualFormatEndingLength = strlen(FILE_FORMAT_ENDING);
  char *lastSymbols = (char *)calloc(actualFormatEndingLength, sizeof(char));
  for (int i=0,j=(strlen(fileByteArray) - actualFormatEndingLength) ; i<actualFormatEndingLength ; i++,j++) {
	lastSymbols[i] = fileByteArray[j];
  }
  
  if (strcmp(lastSymbols, FILE_FORMAT_ENDING) != 0) {
	printf("%s\n", FILE_FORMAT_ERROR_ENDING);
	free(lastSymbols);
	return FALSE;
  }
  free(lastSymbols);
  
  // Jeigu viskas gerai, gražiname TRUE
  return TRUE;
}

/**
 * Nuskaito failą ir gražina simbolių masyvą. 
 */
char *init_char_array_from_file(const char *fileName) {
  // Atidarome failą
  FILE *osFile;
  osFile = fopen(fileName, "r");
  if (osFile == NULL) {
	printf("%s\n", FILE_DOES_NOT_EXIST);
    return NULL;
  }
  
  // Einame į failo pabaigą, paskaičiuojame ilgį
  fseek(osFile, 0L, SEEK_END);
  int elements = ftell(osFile);
  
  // Sukuriame char masyvą tolygų failo dydžiui
  char *dataToReturn = (char *)calloc(elements, sizeof(char));
  
  // Grįžtame į failo pradžią
  fseek(osFile, 0L, SEEK_SET);

  // Nuskaitome visus char'us iš failo į masyvą
  int charCounter = 0;
  char readChar;
  while ((readChar = getc(osFile)) != EOF) {
    dataToReturn[charCounter++] = readChar;
  }
  
  // Uždarome failą
  fclose(osFile);
  
  // Tikriname, ar failas atitinka formatą
  if (is_file_of_required_format(dataToReturn) == FALSE) {
	return NULL;
  }
  
  // Gražiname rezultatą
  return dataToReturn;
}

/**
 * Gražina tekstinę eilutę nurodytuose rėžiuose.
 */
char *substring_from_to(const char *dataLine, const int from, const int to) {
  char *arrayToReturn = (char *)calloc(to - from + 2, sizeof(char));

  for (int i=from,j=0 ; i<=to ; i++,j++) {
	arrayToReturn[j] = dataLine[i];
	
	if (i == to) {
	  arrayToReturn[j+1] = '\0';
	}
  }

  return arrayToReturn;
}

/**
 * Gražina tekstinę eilutę nuo nurodyto rėžio iki nurodyto simbolio.
 */
char *substring_from_until_symbol(const char *dataLine, const int from, const char endSymbol, const int maxLength) {
  char *arrayToReturn = (char *)calloc(maxLength, sizeof(char));
  int dataArrayIteratorIndex = from;
  int newArrayIteratorIndex = 0;
  
  do {
    char symbol = dataLine[dataArrayIteratorIndex];
	if (symbol == endSymbol) {
	  break;
	}
	
	arrayToReturn[newArrayIteratorIndex++] = symbol;
	dataArrayIteratorIndex++;
	
  } while (dataArrayIteratorIndex > 0 && dataArrayIteratorIndex <= maxLength);
  arrayToReturn[newArrayIteratorIndex] = '\0';
  
  arrayToReturn = (char *)realloc(arrayToReturn, newArrayIteratorIndex);
  return arrayToReturn;
}

/**
 *
 */
int initialize_program_data_to_memory(const int maximumLinesToPrint, const char *programName) {
  // TODO
  return TRUE;
}

/**
 *
 */
int initialize_program_code_to_memory(char programCodeLines[][FILE_FORMAT_WORD_LENGTH + 1], int linesLength) {
  for (int i=0 ; i<linesLength ; i++) {
    // TODO
	printf("%s\n", programCodeLines[i]);
  }
  return TRUE;
}

/**
 * Pagal įvesties duomenis, suvedame visus duomenis į atmintį
 */
int initialize_given_program_to_memory(const char *fileByteArray) {
  // Išgauname maksimalų spausdinamų eilučių kiekį
  const int maximumLinesToPrint = atoi(substring_from_to(fileByteArray, FILE_FORMAT_PROGRAM_MAX_LINES_FROM, 
    FILE_FORMAT_PROGRAM_MAX_LINES_TO));
  if (maximumLinesToPrint == 0) {
    printf("%s\n", FILE_FORMAT_ERROR_INCORRECT_LINES_LENGTH);
    return FALSE;
  }
	
  // Išgauname programos pavadinimą
  const char *programName = substring_from_until_symbol(fileByteArray, FILE_FORMAT_PROGRAM_NAME_FROM, 
    FILE_FORMAT_PROGRAM_NAME_END_SYMBOL, FILE_FORMAT_PROGRAM_NAME_LENGTH);

  if (programName == NULL) {
    printf("%s\n", FILE_FORMAT_ERROR_INCORRECT_NAME);
    return FALSE;
  }
  
  // Įrašome į atmintį
  int initializedData = initialize_program_data_to_memory(maximumLinesToPrint, programName);
  if (initializedData == FALSE) {
	printf("%s\n", MEMORY_ERROR_DATA_FAILURE);
    return FALSE;
  }
  
  // Išgauname pačią programą į simbolių masyvą
  int programCodeLength = strlen(fileByteArray) - strlen(FILE_FORMAT_BEGINNING) - 
    strlen(FILE_FORMAT_ENDING) - FILE_FORMAT_PROGRAM_MAX_LINES_FROM - strlen(programName) - 1;
  
  if (programCodeLength % FILE_FORMAT_WORD_LENGTH != 0) {
	printf("%s\n", FILE_FORMAT_WORD_LENGTH_INCORRECT);
	return FALSE;
  }
  
  int programCodeStartIndex = strlen(FILE_FORMAT_BEGINNING) + 
    (FILE_FORMAT_PROGRAM_MAX_LINES_TO - FILE_FORMAT_PROGRAM_MAX_LINES_FROM + 1) +
	strlen(programName) + 1;

  const char* programCode = substring_from_to(fileByteArray, programCodeStartIndex, 
    programCodeStartIndex + programCodeLength - 1);

  // Iš programos simbolių masyvo, sukuriame programos žodžių 2d masyvą
  const int wordsAmount = programCodeLength / FILE_FORMAT_WORD_LENGTH;
  char programCodeLines[wordsAmount][FILE_FORMAT_WORD_LENGTH + 1];
  
  int lineNo = 0;
  for (int i=0 ; i<strlen(programCode) ; i=i+FILE_FORMAT_WORD_LENGTH) {
    for (int j=0 ; j<FILE_FORMAT_WORD_LENGTH ; j++) {
	  programCodeLines[lineNo][j] = programCode[i+j];
	}

	programCodeLines[lineNo][FILE_FORMAT_WORD_LENGTH] = '\0';
	lineNo++;
  }
  
  // Į atmintį įrašome visas kodų komandas 
  int initializedCode = initialize_program_code_to_memory(programCodeLines, lineNo);
  if (initializedCode == FALSE) {
    printf("%s\n", MEMORY_ERROR_CODE_FAILURE);
	return FALSE;
  }

  // Jeigu neįvyko problemų - gražiname sėkmės reikšmę
  return TRUE;
}

/**
 * Pradeda darbą. 
 */
int main(int argc, const char *argv[]) {
  // Tikriname vartotojo įvestus parametrus
  if (is_argument_data_correct(argc, argv) == FALSE) {
    printf("%s\n", INCORRECT_PARAMETERS);
	wait_for_user_interaction();
	return EXIT_FAILURE;
  }
  
  // Nuskaitome failo duomenis į baitų masyvą
  const char *fileByteArray = init_char_array_from_file(argv[1]);
  if (fileByteArray == NULL) {
    wait_for_user_interaction();
	return EXIT_FAILURE;
  }
	
  // Įdiegiame operacinės sistemos atmintį
  initialize_memory();
  
  // Įdiegiame PTR registrą su nuorodą į puslapiavimo lentelę
  initialize_ptr();

  // Įdiegiame puslapiavimo lentelę
  initialize_page_table();
  
  // Įdiegiame pradinę SP reikšmę
  initialize_sp();
  
  // Įdiegiame pradinę PC reikšmę
  initialize_pc();
  
// TODO
  print_page_table();
  int realAddress = get_real_word_address(185);
  printf("\n\nVirtualus zodzio adresas - 185. O realus - %d\n", realAddress);
// TODO
  
  // Inicializuojame programą į atmintį
  int flowSuccess = initialize_given_program_to_memory(fileByteArray);
  if (flowSuccess == FALSE) {
	wait_for_user_interaction();
	return EXIT_FAILURE;
  }
  
  // Užbaigiame darbą
  wait_for_user_interaction();
  return EXIT_SUCCESS;
}