#ifndef HELPER
#define HELPER
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "VCParser.h"
#include "LinkedListAPI.h"

#define PROP_NAMES(...) const char *FOO[] = { __VA_ARGS__ }

char *readFile(char *filename, int* passed);

List *tokenize(char *content, char *delimiter);

char *print(void *pointer);

int compare(const void *a, const void *b);

char *stringCopy(char *input, int start, int end);

List *tokenize(char *content, char *delimiter);

List* split(char* s, char* delim);


Parameter *newParameter(char *name, char *value);

Property *newProperty(char *name, List *values, List *parameters, char *group);

Property* parseProperty(char* s);
char* parseGroup(char* s);
void parseNameAndGroup(char* s, char** name, char** group);

Parameter* parseParameter(char* p);

DateTime *newDateTime(bool UTC, bool isText, char *date, char *time, char *text);

DateTime* parseDateTime(char* dt);

Card *newCard(Property *fn, List *optionalProperties, DateTime *birthday, DateTime *anniversary);

Property* newPropertyWrapper(char* propertyLine);

DateTime* newDateTimeWrapper(char* line);

int getIndex(char* s, char c);

char* unfold(char* line1, char* line2);

List* foldOrganize(List* list);

//Assignment 2 

char* writeParameter(Parameter* param);

char* writeProperty(Property* prop);

char* writeDate(DateTime* date, char* dateType);

char* removeChar(char* s, char toRemove);

int trimIndex(char* string);

List* valSplit(char* vals);

char* getValuefromJSON(char* string);

char* paramToJSON(Parameter* p);

char* cardToJSON(Card* obj);

char* propListToJSON(List* props);

char* fileToJSON(char* filename);

char* addPropToFile(char* filename, char* propJSON);


char* createCardFromJSON(char* filename, char* json, char* props, char* birth, char* anni);
/**********HELPER FUNCTIONS FOR VALIDATING**************/

VCardErrorCode validPropName(char* name);
VCardErrorCode validateProp(const void* p);
VCardErrorCode validateValue(char* value);
VCardErrorCode validateParameter(const void* parameter);
VCardErrorCode validateDateTime(const void* dateTime);
VCardErrorCode validateDate(char* date);
VCardErrorCode validateTime(char* time);
// VCardErrorCode validateCardinality(Card* card);
char* updateValue(char* filename, char* currentJSON, char* newJSON);

#endif
