#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "VCParser.h"
#include "LinkedListAPI.h"
#include "VChelper.h"

//function to read file
char *readFile(char *filename)
{
    if (filename == NULL)
        return NULL;
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0L, SEEK_END);
    unsigned long lSize = ftell(fp);
    rewind(fp);
    char *filecontent = calloc(lSize + 10, 1);
    for (int i = 0; i < lSize; i++)
    {
        char c = fgetc(fp);
        if (feof(fp))
            break;
        filecontent[i] = c;
    }
    fclose(fp);
    return filecontent;
}

int compare(const void *a, const void *b)
{
    return 0;
}

//function to copy string
char *stringCopy(char *input, int start, int end)
{
    int space = (end - start) + 10;
    char *copy = calloc(space, 1);
    for (int i = start; i < end; i++)
    {
        copy[i - start] = input[i];
    }
    return copy;
}

//function to tokenize a string
List *tokenize(char *content, char *delimiter)
{
    List *list = initializeList(&print, &free, &compare);
    char *cpy = stringCopy(content, 0, strlen(content));
    char *token;
    token = strtok(cpy, delimiter);
    while (token != NULL)
    {
        char *copy = stringCopy(token, 0, strlen(token));
        insertBack(list, copy);
        token = strtok(NULL, delimiter);
    }
    free(cpy);
    return list;
}

List *split(char *s, char *delim)
{
    if (s == NULL)
    {
        return initializeList(print, free, compare);
    }
    else if (strlen(s) == 0)
    {
        return initializeList(print, free, compare);
    }
    else
    {
        char *t = s;
        char *r = t;
        char *tempString = strstr(t, delim);
        if (tempString == NULL)
        {
            List *l = initializeList(print, free, compare);
            insertFront(l, stringCopy(r, 0, strlen(r)));
            return l;
        }
        else
        {
            s += (tempString - s) + strlen(delim);
            List *l = split(s, delim);
            insertFront(l, stringCopy(r, 0, tempString - r));
            return l;
        }
    }
}

//copies a string without the character toRemove
char* removeChar(char* s, char toRemove)
{
    char* ss = calloc(strlen(s)+1, 1);
    int c = 0;
    for(int i = 0; i < strlen(s); i++)
    {
        if(s[i] != toRemove)
        {
            ss[c] = s[i];
            c++;
        }
    }
    return ss;
}

//Constructors

Parameter *newParameter(char *name, char *value)
{
    Parameter *p = calloc(1, sizeof(Parameter));
    p->name = stringCopy(name, 0, strlen(name));
    p->value = stringCopy(value, 0, strlen(value));
    return p;
}

Parameter *parseParameter(char *p)
{
    int equalIndex = getIndex(p, '=');
    if (equalIndex == -1 || equalIndex == 0 || equalIndex == strlen(p))
        return NULL;
    List *parameters = tokenize(p, "=");
    char *name = (char *)parameters->head->data;
    char *value = (char *)parameters->head->next->data;
    Parameter *pp = newParameter(name, value);
    freeList(parameters);
    return pp;
}

Property *newProperty(char *name, List *values, List *parameters, char *group)
{
    Property *p = calloc(1, sizeof(Property));
    p->name = stringCopy(name, 0, strlen(name));
    p->group = stringCopy(group, 0, strlen(group));
    p->parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);
    for (Node *node = parameters->head; node != NULL; node = node->next)
    {
        Parameter *OrigParameter = node->data;
        Parameter *newPrmtr = newParameter(OrigParameter->name, OrigParameter->value);
        insertBack(p->parameters, newPrmtr);
    }
    p->values = initializeList(&valueToString, &deleteValue, &compareValues);
    for (Node *node = values->head; node != NULL; node = node->next)
    {
        char *OrigValue = node->data;
        char *newValue = stringCopy(OrigValue, 0, strlen(OrigValue));
        insertBack(p->values, newValue);
    }
    return p;
}

char *parseGroup(char *s)
{
    int index = getIndex(s, '.');
    char *ss = NULL;
    if (index != -1)
    {
        ss = stringCopy(s, 0, index);
    }
    return ss;
}

void parseNameAndGroup(char *s, char **name, char **group)
{
    char *temp = parseGroup(s);
    if (temp == NULL)
    {
        *name = stringCopy(s, 0, strlen(s));
        *group = calloc(10, sizeof(char));
    }
    else
    {
        *group = stringCopy(temp, 0, strlen(temp));
        free(temp);
        int index = getIndex(s, '.');
        *name = stringCopy(s, index + 1, strlen(s));
    }
}

Property *parseProperty(char *s)
{
    char *name = NULL;
    char *group = NULL;
    int colonIndex = -1;
    int semiColonIndex = -1;
    colonIndex = getIndex(s, ':');
    semiColonIndex = getIndex(s, ';');
    char *vals = stringCopy(s, colonIndex + 1, strlen((char *)s));
    List *values = tokenize(vals, ";");
    free(vals);
    List *parameters = initializeList(parameterToString, deleteParameter, compareParameters);
    if (semiColonIndex != -1)
    {
        if (semiColonIndex < colonIndex)
        {
            char *nameGroup = stringCopy(s, 0, semiColonIndex);
            parseNameAndGroup(nameGroup, &name, &group);
            free(nameGroup);
            char *parameterString = stringCopy(s, semiColonIndex + 1, colonIndex);
            List *parameterStrings = tokenize(parameterString, ";");
            for (Node *n = parameterStrings->head; n != NULL; n = n->next)
            {
                Parameter* param = parseParameter(n->data);
                if(param == NULL)
                {
                    freeList(values);
                    freeList(parameters);
                    free(name);
                    free(group);
                    freeList(parameterStrings);
                    return NULL;
                }
                insertBack(parameters, param);
            }
            free(parameterString);
            freeList(parameterStrings);
        }
        else
        {
            char *nameGroup = stringCopy(s, 0, colonIndex);
            parseNameAndGroup(nameGroup, &name, &group);
            free(nameGroup);
        }
    }
    else
    {
        char *nameGroup = stringCopy(s, 0, colonIndex);
        parseNameAndGroup(nameGroup, &name, &group);
        free(nameGroup);
    }
    Property *prop = newProperty(name, values, parameters, group);
    free(name);
    freeList(values);
    freeList(parameters);
    free(group);
    return prop;
}

DateTime *newDateTime(bool UTC, bool isText, char *date, char *time, char *text)
{
    DateTime *d = calloc(1, sizeof(DateTime));
    d->UTC = UTC;
    d->isText = isText;
    d->date = stringCopy(date, 0, strlen(date));
    d->time = stringCopy(time, 0, strlen(time));
    d->text = stringCopy(text, 0, strlen(text));
    return d;
}

DateTime *parseDateTime(char *dt)
{
    int tIndex = getIndex(dt, 'T');
    char *date = calloc(10, 1);
    char *time = calloc(10, 1);
    char *text = calloc(10, 1);
    bool isText = false;
    bool isUTC = false;
    int zIndex = getIndex(dt, 'Z');
    if (zIndex == -1 && tIndex == -1)
    {
        free(text);
        text = stringCopy(dt, 0, strlen(dt));
        isText = true;
        DateTime *dt2 = newDateTime(isUTC, isText, date, time, text);
        free(date);
        free(time);
        free(text);
        return dt2;
    }
    int length = 0;
    if (zIndex != -1)
    {
        length = strlen(dt) - 1;
        isUTC = true;
    }
    else
        length = strlen(dt);
    if (tIndex == -1)
    {
        free(date);
        date = stringCopy(dt, 0, length);
    }
    else if (tIndex == 0)
    {
        free(time);
        time = stringCopy(dt, 1, length);
    }
    else
    {
        free(time);
        free(date);
        date = stringCopy(dt, 0, tIndex);
        time = stringCopy(dt, tIndex + 1, length);
    }
    DateTime *dt3 = newDateTime(isUTC, isText, date, time, text);
    free(date);
    free(time);
    free(text);
    return dt3;
}

Card *newCard(Property *fn, List *optionalProperties, DateTime *birthday, DateTime *anniversary)
{
    Card *c = calloc(1, sizeof(Card));
    c->fn = newProperty(fn->name, fn->values, fn->parameters, fn->group);
    c->birthday = newDateTime(birthday->UTC, birthday->isText, birthday->date, birthday->time, birthday->text);
    c->anniversary = newDateTime(anniversary->UTC, anniversary->isText, anniversary->date, anniversary->time, anniversary->text);
    c->optionalProperties = initializeList(&propertyToString, &deleteProperty, &compareProperties);
    for (Node *node = optionalProperties->head; node != NULL; node = node->next)
    {
        Property *OrigProperty = node->data;
        Property *newPrprty = newProperty(OrigProperty->name, OrigProperty->values, OrigProperty->parameters, OrigProperty->group);
        insertBack(c->optionalProperties, newPrprty);
    }
    return c;
}

Property *newPropertyWrapper(char *propertyLine)
{
    Property *newProp = NULL;
    List *lineContent = tokenize(propertyLine, ":");

    List *vals = initializeList(&print, &free, &compare);
    //right of first colon, getting values
    for (Node *node = lineContent->head->next; node != NULL; node = node->next)
    {

        //splitting values based on comma
        List *values = tokenize(node->data, ",");

        for (Node *nodeV = values->head; nodeV != NULL; nodeV = nodeV->next)
        {
            insertBack(vals, stringCopy(nodeV->data, 0, strlen(nodeV->data)));
        }
        freeList(values);
    }

    List *semicolonParse = tokenize(lineContent->head->data, ";");

    //get Group
    int index = getIndex(semicolonParse->head->data, '.');
    char *group = calloc(20, 1);
    char *name = NULL;

    //get name & group
    if (index != -1)
    {
        group = strncpy(group, semicolonParse->head->data, index);
        name = stringCopy(semicolonParse->head->data, index + 1, strlen(semicolonParse->head->data));
    }
    else
    {
        name = stringCopy(semicolonParse->head->data, 0, strlen(semicolonParse->head->data));
    }

    List *parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);

    //Getting parameters
    for (Node *nodeP = semicolonParse->head->next; nodeP != NULL; nodeP = nodeP->next)
    {
        int index = getIndex(nodeP->data, '=');
        char *value = stringCopy(nodeP->data, index + 1, strlen(nodeP->data));
        char *name = stringCopy(nodeP->data, 0, index);
        Parameter *newParam = newParameter(name, value);
        free(name);
        free(value);
        insertBack(parameters, newParam);
    }
    if (strcmp(name, "BEGIN") != 0 && strcmp(name, "VERSION") != 0 && strcmp(name, "BDAY") != 0 && strcmp(name, "ANNIVERSARY") != 0 && strcmp(name, "END") != 0)
    {
        newProp = newProperty(name, vals, parameters, group);
    }

    free(name);
    free(group);
    freeList(vals);
    freeList(parameters);
    freeList(lineContent);
    freeList(semicolonParse);

    return newProp;
}

//function to give index of a certain character (use to find '=' for splitting parameter into name & value)
int getIndex(char *s, char c)
{
    for (int i = 0; i < strlen(s); i++)
    {
        if (s[i] == c)
            return i;
    }
    return -1;
}

//function to allow for list of strings
char *print(void *pointer)
{
    char *temp = (char *)pointer;
    char *copy = stringCopy(temp, 0, strlen(temp));
    return copy;
}

//function to join 2 folded lines into single unfolded line
char *unfold(char *line1, char *line2)
{
    line1 = realloc(line1, strlen(line1) + strlen(line2) + 10);
    strcat(line1, line2);
    return line1;
}

//function to iterate through all the list of lines, and reorganize folded linee
List *foldOrganize(List *list)
{
    List *list2 = initializeList(&print, &free, &compare);
    for (Node *n = list->head; n != NULL; n = n->next)
    {
        char *s = (char *)(n->data);
        if (s[0] == 32 || s[0] == 9)
        {

            n->previous->data = unfold(n->previous->data, n->data);
        }
    }
    for (Node *n = list->head; n != NULL; n = n->next)
    {
        char *s = n->data;
        if (s[0] != 32 && s[0] != 9)
        {
            insertBack(list2, stringCopy(s, 0, strlen(s)));
        }
    }
    return list2;
}

DateTime *newDateTimeWrapper(char *line)
{
    DateTime *dt = NULL;
    List *dtList = tokenize(line, "T");
    if (dtList->length == 2)
    {
        dt = newDateTime(false, true, dtList->head->data, dtList->head->next->data, "");
    }
    else
    {
        dt = newDateTime(false, true, dtList->head->data, "", "");
    }
    freeList(dtList);

    return dt;
}

//************************Assignment 2 functions****************

char *writeParameter(Parameter *param)
{
    char *s = calloc(100, 1);
    strcat(s, param->name);
    strcat(s, "=");
    strcat(s, param->value);
    return s;
}

char *writeProperty(Property *prop)
{
    char *parameterList = toString(prop->parameters);
    char *valueList = toString(prop->values);
    char *s = calloc(200 + strlen(parameterList) + strlen(valueList), 1);
    if (strlen(prop->group) > 0)
    {
        strcat(s, prop->group);
        strcat(s, ".");
        strcat(s, prop->name);
    }
    else
        strcat(s, prop->name);
    if (prop->parameters->length == 0)
        strcat(s, ":");
    else
    {
        strcat(s, ";");
        for (Node *n = prop->parameters->head; n != NULL; n = n->next)
        {
            char *writeParam = writeParameter(n->data);
            strcat(s, writeParam);
            free(writeParam);
            if (n == prop->parameters->tail)
            {
                strcat(s, ":");
            }
            else
                strcat(s, ";");
        }
    }
    for (Node *n = prop->values->head; n != NULL; n = n->next)
    {
        char *writeValue = valueToString(n->data);
        strcat(s, writeValue);
        free(writeValue);
        if (n == prop->values->tail)
        {
            strcat(s, "\r\n");
        }
        else
            strcat(s, ",");
    }
    free(parameterList);
    free(valueList);
    return s;
}

char *writeDate(DateTime *date, char *dateType)
{
    char *s = calloc(300, 1);
    strcat(s, dateType);
    if (date->isText == true)
    {
        strcat(s, ";VALUE=text:");
        strcat(s, date->text);

        strcat(s, "\r\n");
        return s;
    }
    if (strlen(date->date) > 0 && strlen(date->time) > 0)
    {
        strcat(s, date->date);
        strcat(s, "T");
        strcat(s, date->time);
    }
    else if (strlen(date->date) != 0)
        strcat(s, date->date);
    else
        strcat(s, date->time);
    if (date->UTC == true)
        strcat(s, "Z");
    strcat(s, "\r\n");
    return s;
}



/**********HELPER FUNCTIONS FOR VALIDATING**************/

VCardErrorCode validPropName(char* name)
{
    if(name == NULL)return INV_PROP;
    if(strlen(name) <= 0)return INV_PROP;
    char* PROP_NAMES [] =  {"SOURCE","KIND","XML","FN","N","NICKNAME","PHOTO","GENDER","ADR",
    "TEL","EMAIL","IMPP","LANG","TZ","TITLE","ROLE","LOGO","ORG","MEMBER","RELATED",
    "CATEGORIES","NOTE","PRODID","REV","SOUND","UID","CLIENTPIDMAP","URL","KEY","FBURL",
    "CALADRURI","CALURI", NULL};

    int i = 0;
    while(PROP_NAMES[i] != NULL)
    {
        if(strcmp(name, PROP_NAMES[i]) == 0)
        {
            return OK;
        }
        i++;
    }
    return INV_PROP;
}


VCardErrorCode validateProp(const void* p)
{
    if(p == NULL)return INV_PROP;
    Property* prop = (Property*)p;


    //validate the name
    if(validPropName(prop->name) != OK)return INV_PROP;
    
    //NULL check
    if(prop->group == NULL)return INV_PROP;
    
    //validate the 
    if(prop->values == NULL)return INV_PROP;
    if(prop->parameters == NULL)return INV_PROP;
    
    //check each value
    for(Node* n = prop->values->head; n != NULL; n = n->next)
    {
        if(validateValue(n->data) != OK)return INV_PROP;
    }


    //check each paramter
    for(Node* n = prop->parameters->head; n != NULL; n = n->next)
    {
        if(validateParameter(n->data) != OK)return INV_PROP;
    }

    //make sure it has at least one value
    if(getLength(prop->values) == 0)return INV_PROP;
    
    //N and 5 required
    if((strcmp(prop->name, "N")==0)&& getLength(prop->values) != 5)return INV_PROP;


    return OK;
}

VCardErrorCode validateValue(char* value)
{
    if(value == NULL)return INV_PROP;
    return OK;
}

VCardErrorCode validateParameter(const void* parameter)
{
    if(parameter == NULL)return INV_PROP;
    Parameter* param = (Parameter*)parameter;
    if(param->name == NULL || param->value == NULL)return INV_PROP;
    if(!strlen(param->name) || !strlen(param->value))return INV_PROP;
    return OK;
}

//validate a DT
VCardErrorCode validateDateTime(const void* dateTime)
{
    if(dateTime == NULL)return OK;
    DateTime* dt = (DateTime*)dateTime;
    if(dt->isText && dt->UTC)return INV_DT;

    //if any are null
    if(dt->date == NULL || dt->text == NULL || dt->time == NULL)return INV_DT;
    //if all are empty
    if(!strlen(dt->text) && !strlen(dt->time) && !strlen(dt->date))return INV_DT;

    //if text and time or date > 0
    if(strlen(dt->text)> 0 && (strlen(dt->time)>0 || strlen(dt->date)>0))return INV_DT;

    //check of the text is messed up
    if(dt->isText && (strlen(dt->time)>0 || strlen(dt->date)>0))return INV_DT;
    if(!(dt->isText) && strlen(dt->text)>0)return INV_DT;

    if(dt->isText && dt->UTC)return INV_DT;

    if(dt->isText && !strlen(dt->text))return INV_DT;

    VCardErrorCode ec = OK;


    if(!(dt->isText))
    {
        char* date = dt->date;
        char* time = dt->time; 
        if(!strlen(date) && !(strlen(time)))return INV_DT;

        ec = validateDate(date);
        if(ec != OK)return ec;
        ec = validateTime(date);
        if(ec != OK)return ec;
    }
    return ec;
}

VCardErrorCode validateDate(char* date)
{
    if(date == NULL)return INV_DT;
    if(!strlen(date))return OK;
    int idx = getIndex(date, '-');
    VCardErrorCode ec = OK;
    if(idx == -1)
    {
        char* y = stringCopy(date, 0, 4);
        char* m = stringCopy(date, 4, 6);
        char* d = stringCopy(date, 6, 8);
        int year = atoi(y);
        if(year < 0)ec = INV_DT;
        int month = atoi(m);
        if(month < 1 || month > 12)ec =  INV_DT;
        int day = atoi(d);
        if(day  < 1 || day > 31)ec = INV_DT;
        free(y);free(m);free(d);
        //all three
    }
    else if(idx == 0)
    {
        //month and day or just day
        char* m = stringCopy(date, 2, 4);
        char* d = stringCopy(date, 4, 6);
        int month = atoi(m);
        if(month < 1 || month > 12)ec = INV_DT;
        int day = atoi(m);
        if(day  < 1 || day > 31)ec = INV_DT;
        free(m);
        free(d);


    }
    else if(idx == 4)
    {
        char* y = stringCopy(date, 0, 4);
        char* d = stringCopy(date, 5, 7);
        int year = atoi(y);
        if(year < 0)ec = INV_DT;
        int day = atoi(d);
        if(day  < 1 || day > 31)ec = INV_DT;



        //year and day
    }
    else 
    {
        return ec = INV_DT;
    }

    //return the error code
    return ec;

}

VCardErrorCode validateTime(char* time)
{
    if(time == NULL)return INV_DT;
    if(!strlen(time))return OK;
    
    int idx = getIndex(time, '-');
    VCardErrorCode ec = OK;
    //no dash
    if(idx == -1)
    {
        int len = strlen(time);
        //all three
        if(len == 6)
        {
            char* s = stringCopy(time, 0, 2);
            int t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
            s = stringCopy(time, 2, 4);
            t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
            s = stringCopy(time, 4, 6);
            t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);

        }
        //jsut two 
        else if(len == 4)
        {
            char* s = stringCopy(time, 0, 2);
            int t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
            s = stringCopy(time, 2, 4);
            t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
        }
        //just 1
        else if(len == 2)
        {
            char* s = stringCopy(time, 0, 2);
            int t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
        }
        else 
        {
            ec = INV_DT;
        }
    }//end if no dash
    
    //if dash
    else 
    {
        char* s = removeChar(time, '-');
        int len = strlen(s);
        //if just one
           //jsut two 
        if(len == 4)
        {
            char* s = stringCopy(time, 0, 2);
            int t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
            s = stringCopy(time, 2, 4);
            t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
        }
        else if(len == 2)
        {
            char* s = stringCopy(time, 0, 2);
            int t = atoi(s);
            if(t < 0 || t > 60)ec = INV_DT;
            free(s);
        }
        else 
        {
            ec = INV_DT;
        }
    }

    return ec;

}


// VCardErrorCode validateCardinality(Card* card)
// {
//     int nCount = 0;
// }
