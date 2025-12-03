/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* maximum number of scopes */
#define MAX_SCOPE 1000


/* the list of line numbers of the source 
* code in which a variable is referenced
*/
typedef struct LineListRec
{ int lineno;
    struct LineListRec * next;
} * LineList;

/* The record in the bucket lists for
* each variable, including name, 
* assigned memory location, and
* the list of line numbers in which
* it appears in the source code
*/
typedef struct BucketListRec
{ char * name;
    LineList lines;
    int memloc ; /* memory location for variable */
  struct treeNode * decl; /* pointer to declaration node (for functions/arrays) */
  int isFunc; /* non-zero if this bucket represents a function */
    struct BucketListRec * next;
} * BucketList;

static BucketList hashTable[SIZE];

/* the record for each scope */
typedef struct ScopeRec
{ char * funcName;
    int nestedLevel;
    struct ScopeRec * parent;
    BucketList hashTable[SIZE]; /* the hash table */
} * Scope;

/* Procedure st_insert inserts line numbers and
* memory locations into the symbol table
* loc = memory location is inserted only the
* first time, otherwise ignored
*/
void st_insert( char * name, int lineno, int loc);

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name );

int st_lookup_top ( char * name );

/* Function st_bucket returns the BucketList
 * entry for a name
 */
BucketList getBucketList(char * name);

/* Add line number to existing symbol (searches in all scopes) */
void st_addLineNumber(char *name, int lineno);

/* Scope stack operations */
Scope createScope(char *funcName);
Scope currentScope();
void popScope();
void pushScope(Scope scope);

/* Location management for variables */
int addLocation();

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);


#endif
