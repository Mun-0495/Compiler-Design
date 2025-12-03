/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

static Scope scopes[MAX_SCOPE];
static Scope scopeStack[MAX_SCOPE];
static int location[MAX_SCOPE];

/* number of current scopes */
static int nScope = 0;
static int nScopeStack = 0;

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* Initialize hashTable entries to NULL */
static void initHashTable() {
    int i;
    for (i = 0; i < SIZE; ++i) hashTable[i] = NULL;
}

void pushScope(Scope scope) {
    scopeStack[nScopeStack] = scope;
    location[nScopeStack++] = 0;
}

void popScope() {
    if (nScopeStack > 0) {
        nScopeStack--;
    }
}

Scope currentScope() {
    if (nScopeStack > 0) {
        return scopeStack[nScopeStack - 1];
    }
    return NULL;
}

Scope createScope(char * funcName) {
  if(nScope == 0) initHashTable();

  Scope newScope = (Scope) malloc(sizeof(struct ScopeRec));
  newScope->funcName = funcName;
  newScope->nestedLevel = nScopeStack;
  newScope->parent = currentScope();

  /* initialize the scope's internal hash table */
  // int i;
  // for (i = 0; i < SIZE; ++i) newScope->hashTable[i] = NULL;

  scopes[nScope++] = newScope;
  return newScope;
}

int addLocation() {
  if (nScopeStack <= 0) return 0; /* 또는 -1로 에러 처리 */
  return location[nScopeStack - 1]++;
}

BucketList getBucketList(char * name) {
    Scope scope = currentScope();
    while (scope != NULL) {
        int h = hash(name);
        BucketList l = scope->hashTable[h];
        while (l != NULL && strcmp(name, l->name) != 0) l = l->next;
        if (l != NULL) return l;
        scope = scope->parent;
    }
    /* not found */
    return NULL;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc)
{  int h = hash(name);
  Scope scope = currentScope();
  BucketList l = scope->hashTable[h];

  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->lines->next = NULL;
    l->memloc = loc;
    l->isFunc = 0;
    l->next = scope->hashTable[h];
    scope->hashTable[h] = l;
  }
  else /* found in table, so just add line number */
  {
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
}

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{ 
  Scope s = currentScope();
  while (s != NULL) {
    int h = hash(name);
    BucketList l = s->hashTable[h];
    while (l != NULL && strcmp(name, l->name) != 0) l = l->next;
    if (l != NULL) return l->memloc;
    s = s->parent;
  }
  return -1;
}

/* Get Symbol lookup Top */
int st_lookup_top ( char * name )
{ 
  Scope sc = currentScope();
  if (sc == NULL) return -1;
  int h = hash(name);
  BucketList l = sc->hashTable[h];
  while (l != NULL && strcmp(name, l->name) != 0) l = l->next;
  if (l != NULL) return l->memloc;
  return -1;
}


/* Add line number to existing symbol */
void st_addLineNumber(char *name, int lineno) {
  // /* search scopes from current outward */
  // Scope s = currentScope();
  // while (s != NULL) {
  //   int h = hash(name);
  //   BucketList l = s->hashTable[h];
  //   while ((l != NULL) && (strcmp(name, l->name) != 0)) l = l->next;
  //   if (l != NULL) {
  //     LineList t = l->lines;
  //     while (t->next != NULL) t = t->next;
  //     t->next = (LineList) malloc(sizeof(struct LineListRec));
  //     t->next->lineno = lineno;
  //     t->next->next = NULL;
  //     return;
  //   }
  //   s = s->parent;
  // }
  // /* fallback: search all scopes (global included) */
  // for (int si = 0; si < nScope; ++si) {
  //     Scope sc = scopes[si];
  //     int h = hash(name);
  //     BucketList l = sc->hashTable[h];
  //     while ((l != NULL) && (strcmp(name, l->name) != 0)) l = l->next;
  //     if (l != NULL) {
  //       LineList t = l->lines;
  //       while (t->next != NULL) t = t->next;
  //       t->next = (LineList) malloc(sizeof(struct LineListRec));
  //       t->next->lineno = lineno;
  //       t->next->next = NULL;
  //       return;
  //     }
  // }
  BucketList l = getBucketList(name);
  LineList ll = l->lines;
  while (ll->next != NULL) ll = ll->next;
  ll->next = (LineList) malloc(sizeof(struct LineListRec));
  ll->next->lineno = lineno;
  ll->next->next = NULL;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing,"Scope Name (func)  NestedLevel  Variable Name  Location   Line Numbers\n");
  fprintf(listing,"-----------------  -----------  -------------  --------   ------------\n");
  for (i = 0; i < nScope; ++i) {
      Scope sc = scopes[i];
      const char *scname = sc->funcName ? sc->funcName : "global";
      for (int h = 0; h < SIZE; ++h) {
          BucketList l = sc->hashTable[h];
          while (l != NULL) {
              LineList t = l->lines;
              fprintf(listing,"%-17s  %-11d  %-13s  %-8d  ",
                      scname, sc->nestedLevel, l->name, l->memloc);
              while (t != NULL) {
                  fprintf(listing,"%4d ",t->lineno);
                  t = t->next;
              }
              fprintf(listing,"\n");
              l = l->next;
          }
      }
  }
} /* printSymTab */