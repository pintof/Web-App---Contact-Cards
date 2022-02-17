#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "VCParser.h"
#include "LinkedListAPI.h"
#include "VChelper.h"

// ************* Card parser functions - MUST be implemented ***************
VCardErrorCode createCard(char *fileName, Card **newCardObject)
{

    if (newCardObject == NULL)
        return INV_CARD;
    if (fileName == NULL)
        return INV_FILE;
    if (*newCardObject != NULL)
        return INV_CARD;

    //allocate space for the card
    Card *card = *newCardObject;
    card = calloc(1, sizeof(Card));

    //initialize the fn
    (card)->fn = calloc(1, sizeof(Property));
    (card)->fn->name = calloc(10, sizeof(char));
    (card)->fn->group = calloc(10, sizeof(char));
    (card)->fn->values = initializeList(&valueToString, &deleteValue, &compareValues);
    (card)->fn->parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);

    //initialize the list of other properties

    (card)->optionalProperties = initializeList(&propertyToString, &deleteProperty, &compareProperties);

    //Initialize the date times

    (card)->birthday = NULL;
    (card)->anniversary = NULL;

    //get the content from the file
    char *fileContent = readFile(fileName);

    //tokenize on newline
    List *lines = tokenize(fileContent, "\r\n");
    free(fileContent);

    //fold the lines
    List *folded = foldOrganize(lines);
    freeList(lines);

    for (Node *n = folded->head; n != NULL; n = n->next)
    {
        char *name = NULL;
        char *s = n->data;
        int colonIndex = -1;
        int semiColonIndex = -1;
        colonIndex = getIndex(s, ':');
        if (colonIndex == 0 || colonIndex == -1 || colonIndex == strlen(s))
        {
            freeList(folded);
            deleteCard(card);
            return INV_PROP;
        }
        semiColonIndex = getIndex(s, ';');
        if (semiColonIndex != -1)
            name = stringCopy(s, 0, semiColonIndex);
        else
            name = stringCopy(s, 0, colonIndex);
        if (strcmp(name, "BEGIN") == 0 || strcmp(name, "VERSION") == 0 || strcmp(name, "END") == 0)
        {
        }
        else if (strcmp(name, "FN") == 0)
        {
            deleteProperty(card->fn);
            card->fn = parseProperty((char *)(s));
            if (card->fn == NULL)
            {
                freeList(folded);
                deleteCard(card);
                free(name);
                return INV_PROP;
            }
        }
        else if (strcmp(name, "BDAY") == 0)
        {
            deleteDate(card->birthday);
            char *ss = stringCopy(s, colonIndex + 1, strlen((char *)(s)));
            card->birthday = parseDateTime(ss);
            free(ss);
        }
        else if (strcmp(name, "ANNIVERSARY") == 0)
        {
            deleteDate(card->anniversary);
            char *ss = stringCopy(s, colonIndex + 1, strlen((char *)(s)));
            card->anniversary = parseDateTime(ss);
            free(ss);
        }
        else
        {
            Property *prop = parseProperty(s);
            if (prop == NULL)
            {
                freeList(folded);
                deleteCard(card);
                free(name);
                return INV_PROP;
            }

            insertBack(card->optionalProperties, prop);
        }
        free(name);
    }
    freeList(folded);
    *newCardObject = card;
    return OK;
}
void deleteCard(Card *obj)
{
    if (obj == NULL)
        return;
    deleteProperty(obj->fn);
    freeList(obj->optionalProperties);
    deleteDate(obj->birthday);
    deleteDate(obj->anniversary);
    free(obj);
}
char *cardToString(const Card *obj)
{
    if (obj == NULL)
        return NULL;
    Card *c = (Card *)obj;
    char *op = toString(c->optionalProperties);
    char *p = propertyToString(c->fn);
    char *b = dateToString(c->birthday);
    if (b == NULL)
        b = calloc(10, 1);
    char *a = dateToString(c->anniversary);
    if (a == NULL)
        a = calloc(10, 1);
    char *s = calloc(strlen(op) + strlen(p) + strlen(b) + strlen(a) + 10, 1);
    strcat(s, op);
    strcat(s, p);
    strcat(s, b);
    strcat(s, a);
    free(op);
    free(p);
    free(b);
    free(a);
    return s;
}
char *errorToString(VCardErrorCode err)
{
    return NULL;
}
// *************************************************************************

// ************* List helper functions - MUST be implemented ***************
void deleteProperty(void *toBeDeleted)
{
    if (toBeDeleted == NULL)
        return;
    Property *p = (Property *)toBeDeleted;
    free(p->name);
    free(p->group);
    freeList(p->parameters);
    freeList(p->values);
    free(p);
}
int compareProperties(const void *first, const void *second)
{
    return 0;
}
char *propertyToString(void *prop)
{
    if (prop == NULL)
        return NULL;
    Property *c = (Property *)prop;
    char *parameters = toString(c->parameters);
    char *values = toString(c->values);
    char *s = calloc(strlen(parameters) + strlen(values) + 100, 1);
    strcat(s, "Property: \n");
    strcat(s, c->name);
    strcat(s, "\n");
    strcat(s, c->group);
    strcat(s, "\n");
    strcat(s, parameters);
    strcat(s, "\n");
    strcat(s, values);
    strcat(s, "\n");
    strcat(s, "\n");
    free(parameters);
    free(values);
    return s;
}

void deleteParameter(void *toBeDeleted)
{
    if (toBeDeleted == NULL)
        return;
    Parameter *temp = (Parameter *)toBeDeleted;
    free(temp->name);
    free(temp->value);
    free(temp);
}
int compareParameters(const void *first, const void *second)
{
    return 0;
}
char *parameterToString(void *param)
{
    if (param == NULL)
        return NULL;
    Parameter *temp = (Parameter *)param;
    char *s = calloc(100, 1);
    strcat(s, "Parameter: \n");
    strcat(s, temp->name);
    strcat(s, "\n");
    strcat(s, temp->value);
    strcat(s, "\n");
    strcat(s, "\n");
    return s;
}

void deleteValue(void *toBeDeleted)
{
    if (toBeDeleted == NULL)
        return;
    free(toBeDeleted);
}
int compareValues(const void *first, const void *second)
{
    return 0;
}
char *valueToString(void *val)
{
    if (val == NULL)
        return NULL;
    char *temp = val;
    char *s = calloc(strlen(temp) + 10, 1);
    strcpy(s, temp);
    return s;
}

void deleteDate(void *toBeDeleted)
{
    if (toBeDeleted == NULL)
        return;
    DateTime *d = (DateTime *)toBeDeleted;
    free(d->date);
    free(d->time);
    free(d->text);
    free(d);
}
int compareDates(const void *first, const void *second)
{
    return 0;
}
char *dateToString(void *date)
{
    if (date == NULL)
        return NULL;
    DateTime *d = (DateTime *)date;
    char *s = calloc(500, 1);
    strcat(s, "DATE--TIME \n");
    if (d->UTC == true)
        strcat(s, "UTC = True \n");
    else
        strcat(s, "UTC = False \n");
    if (d->isText == true)
        strcat(s, "isText = True \n");
    else
        strcat(s, "isText = False \n");
    strcat(s, d->date);
    strcat(s, "\n");
    strcat(s, d->time);
    strcat(s, "\n");
    strcat(s, d->text);
    strcat(s, "\n");
    strcat(s, "\n");
    return s;
}
// *************************Assignment 2 Functions*************************

/****************************************MODULE 1: Writing the card to a file**************************************/

VCardErrorCode writeCard(const char *fileName, const Card *obj)
{
    if (fileName == NULL)
        return WRITE_ERROR;
    if (obj == NULL)
        return WRITE_ERROR;

    //open the file
    FILE *file = fopen(fileName, "w");
    fprintf(file, "BEGIN:VCARD\r\n");
    fprintf(file, "VERSION:4.0\r\n");

    //write the fn
    char *s = writeProperty(obj->fn);
    fprintf(file, "%s", s);
    free(s);

    //write the dt
    if (obj->anniversary != NULL)
    {
        s = writeDate(obj->anniversary, "ANNIVERSARY");
        fprintf(file, "%s", s);
        free(s);
    }

    //wrte the dt
    if (obj->birthday != NULL)
    {
        s = writeDate(obj->birthday, "BIRTHDAY");
        fprintf(file, "%s", s);
        free(s);
    }

    //write the optional properteies
    for (Node *n = obj->optionalProperties->head; n != NULL; n = n->next)
    {
        s = writeProperty(n->data);
        fprintf(file, "%s", s);
        free(s);
    }

    //write endVcard and exit
    fprintf(file, "END:VCARD\n");
    fclose(file);
    return OK;
}

/****************************************MODULE 3: Json**************************************/
char *strListToJSON(const List *strList)
{
    if (strList == NULL)
        return NULL;

    List *l = (List *)strList;
    char *json = calloc(l->length * 75, 1);
    strcat(json, "[");
    for (Node *n = strList->head; n != NULL; n = n->next)
    {
        strcat(json, "\"");
        strcat(json, n->data);
        strcat(json, "\"");
        if (n != strList->tail)
            strcat(json, ",");
    }
    strcat(json, "]");
    return json;
}

List *JSONtoStrList(const char *str)
{

    if (str == NULL)
        return NULL;
    //copy str withot the { }
    char *strCopy = stringCopy((char *)str, 1, strlen((char *)str) - 1);
    List *jsonList = split(strCopy, "\",\"");
    free(strCopy);

    List *jsonList2 = initializeList(valueToString, deleteValue, compareValues);

    //copy without the "
    for (Node *n = jsonList->head; n != NULL; n = n->next)
    {
        char *newData = removeChar(n->data, '\"');
        insertBack(jsonList2, newData);
    }

    freeList(jsonList);
    return jsonList2;
}

char *propToJSON(const Property *prop)
{
    if (prop == NULL)
        return NULL;
    Property *p = (Property *)prop;
    char *vals = strListToJSON(p->values);
    char *json = NULL;
    json = calloc(500 + strlen(vals), 1);
    strcat(json, "{");
    //The group
    strcat(json, "\"group\"");
    strcat(json, ":");
    strcat(json, "\"");
    strcat(json, p->group);
    strcat(json, "\"");

    //delimiter
    strcat(json, ",");

    //the name
    strcat(json, "\"name\"");
    strcat(json, ":");
    strcat(json, "\"");
    strcat(json, p->name);
    strcat(json, "\"");

    //delimiter
    strcat(json, ",");

    //the values
    strcat(json, "\"");
    strcat(json, "values\":");

    strcat(json, vals);

    free(vals);
    strcat(json, "}");
    return json;
}

Property *JSONtoProp(const char *str)
{
    if (str == NULL)
        return NULL;

    //parse out the name, group and values
    char *s = (char *)str;

    //parse the values
    int x = getIndex(s, '[');
    int y = getIndex(s, ']');
    char *jsonVals = stringCopy(s, x, y + 1);
    List *vals = JSONtoStrList(jsonVals);
    free(jsonVals);

    List *l = split(s, "\",\"");
    char *g = l->head->data;
    char *n = l->head->next->data;

    //Parse the group
    int idx = getIndex(g, ':');
    char *group = stringCopy(g, idx + 2, strlen(g));

    //parse the name
    idx = getIndex(n, ':');
    char *name = stringCopy(n, idx + 2, strlen(n));

    //create the param
    List *params = initializeList(parameterToString, deleteParameter, compareParameters);
    printf("%s\n%s\n%s\n", name, group, toString(vals));
    Property *prop = newProperty(name, vals, params, group);
    free(name);
    free(group);
    freeList(vals);
    freeList(params);
    freeList(l);
    return prop;
}

char *dtToJSON(const DateTime *prop)
{
    if (prop == NULL)
        return NULL;
    DateTime *dt = (DateTime *)prop;
    char *json = calloc(1000, sizeof(char));
    strcat(json, "{");

    //do the isText portion
    strcat(json, "\"isText\":");
    if (dt->isText)
        strcat(json, "true");
    else
        strcat(json, "false");

    //date
    strcat(json, ",");
    strcat(json, "\"date\":");
    strcat(json, "\"");
    strcat(json, dt->date);
    strcat(json, "\"");

    //time
    strcat(json, ",");
    strcat(json, "\"time\":");
    strcat(json, "\"");
    strcat(json, dt->time);
    strcat(json, "\"");

    //text
    strcat(json, ",");
    strcat(json, "\"text\":");
    strcat(json, "\"");
    strcat(json, dt->text);
    strcat(json, "\"");

    //UTC
    strcat(json, ",");
    strcat(json, "\"isUTC\":");
    if (dt->UTC)
        strcat(json, "true");
    else
        strcat(json, "false");

    strcat(json, "}");

    return json;
}

DateTime *JSONtoDT(const char *str)
{
    if (str == NULL)
        return NULL;
    char *s = (char *)str;
    List *l = split(s, ",");
    printf("%d\n", getLength(l));
    printf("%s\n", toString(l));

    // isText
    char *a = l->head->data;

    //date
    char *d = l->head->next->data;
    //time
    char *t = l->head->next->next->data;
    //text
    char *tx = l->head->next->next->next->data;
    //isUtc
    char *utc = l->head->next->next->next->next->data;

    int idx = getIndex(a, ':');

    char *ss = NULL;
    //get the isText value
    ss = stringCopy(a, idx + 1, strlen(a));
    char *isText = removeChar(ss, '\"');
    free(ss);

    //get the date val
    idx = getIndex(d, ':');
    ss = stringCopy(d, idx + 1, strlen(d));
    char *date = removeChar(ss, '\"');
    free(ss);

    //get the time val
    idx = getIndex(t, ':');
    ss = stringCopy(t, idx + 1, strlen(t));
    char *time = removeChar(ss, '\"');
    free(ss);

    //get the textval
    idx = getIndex(tx, ':');
    ss = stringCopy(tx, idx + 1, strlen(tx));
    char *text = removeChar(ss, '\"');
    free(ss);

    //get the utc val
    idx = getIndex(utc, ':');
    ss = stringCopy(utc, idx + 1, strlen(utc));
    char *utcVal = removeChar(ss, '\"');
    free(ss);

    bool iText = false;
    bool iUTC = false;
    if (!strcmp(isText, "true"))
    {
        iText = true;
    }
    if (!strcmp(utcVal, "true"))
    {
        iUTC = true;
    }
    DateTime *dateTime = newDateTime(iUTC, iText, date, time, text);
    freeList(l);
    free(isText);
    free(date);
    free(time);
    free(text);
    free(utcVal);
    return dateTime;
}

Card *JSONtoCard(const char *str)
{
    if (str == NULL)
        return NULL;

    Card *c = calloc(1, sizeof(Card));
    char *s = (char *)str;
    c->fn = JSONtoProp(s);
    c->anniversary = NULL;
    c->birthday = NULL;
    c->optionalProperties = initializeList(propertyToString, deleteProperty, compareProperties);
    return c;
}

void addProperty(Card *card, const Property *toBeAdded)
{
    if (card == NULL || toBeAdded == NULL)
        return;
    Property *prop = (Property *)toBeAdded;
    if (validPropName(prop->name) == OK)
        insertBack(card->optionalProperties, prop);
}

/**********************************************MODULE 2: VALIDATING*********************************************/
VCardErrorCode validateCard(const Card *obj)
{

    if (obj == NULL)
        return INV_CARD;

    //check the fn

    if (obj->fn == NULL)
        return INV_CARD;

    //check the lists
    if (obj->optionalProperties == NULL)
        return INV_CARD;

    //error code for checking stuff
    VCardErrorCode ec = OK;

    ec = validateProp(obj->fn);
    if (ec != OK)
        return ec;

    //validate date time
    ec = validateDateTime(obj->birthday);
    if (ec != OK)
        return ec;
    ec = validateDateTime(obj->anniversary);
    if (ec != OK)
        return ec;

    int kCount = 0;
    int genCount = 0;
    int nCount = 0;
    int prIdCount = 0;
    int revCount = 0;
    int uIdCount = 0;
    //validate each optiona property
    for (Node *n = obj->optionalProperties->head; n != NULL; n = n->next)
    {
        Property *prop = n->data;
        if (!strcmp(prop->name, "VERSION"))
        {
            return INV_PROP;
        }
        if (!strcmp(prop->name, "FN"))
        {
            return INV_PROP;
        }
        if (!strcmp(prop->name, "KIND"))
        {
            kCount++;
        }
        if (!strcmp(prop->name, "UID"))
        {
            uIdCount++;
        }
        if (!strcmp(prop->name, "N"))
        {
            nCount++;
        }
        if (!strcmp(prop->name, "GENDER"))
        {
            genCount++;
        }
        if (!strcmp(prop->name, "PRODID"))
        {
            prIdCount++;
        }
        if (!strcmp(prop->name, "REV"))
        {
            revCount++;
        }

        ec = validateProp(n->data);
        if (ec != OK)
            return ec;
    }
    if (revCount > 0 || prIdCount > 0 || genCount > 0 || nCount > 0 || kCount > 0 || uIdCount > 0)
        return INV_PROP;


    return OK;
}
