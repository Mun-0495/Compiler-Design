/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer for Cminus compiler           */
/* Supports Undetermined type and function/variable   */
/* declaration/type checking                       */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

// static int location = 0;          /* counter for variable memory locations */
static char *funcName = NULL;
static int preserveLastScope = FALSE;
// int inFuncDecl = FALSE;
Scope globalscope;                /* global scope pointer */

/* Generic syntax tree traversal */
static void traverse(TreeNode *t,
                     void (*preProc)(TreeNode *),
                     void (*postProc)(TreeNode *))
{
    if (t != NULL)
    {
        preProc(t);
        for (int i = 0; i < MAXCHILDREN; i++)
            traverse(t->child[i], preProc, postProc);
        postProc(t);
        traverse(t->sibling, preProc, postProc);
    }
}

/* Error reporting */
static void symbolredifineError(TreeNode *t, char *funcName)
{
  fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line", funcName, t->lineno);
  for (LineList line = getBucketList(funcName)->lines; line != NULL; line = line->next) {
      fprintf(listing, " %d", line->lineno);
  }
  fprintf(listing, ")\n");
  Error = TRUE;
}

/* Insert built-in IO functions */
static void InsertIOFunctions()
{
   TreeNode *inputFunc = newDeclNode(FuncK);
    inputFunc->attr.name = copyString("input");
    inputFunc->lineno = 0;
    inputFunc->type = Integer;
    st_insert("input", 0, addLocation());
    BucketList b = getBucketList("input");
    if (b) { b->isFunc = 1; b->decl = inputFunc; }

    TreeNode *outputFunc = newDeclNode(FuncK);
    TreeNode* param = newParamNode(NonArrParamK);
    param->attr.name = copyString("x");   /* give param a name */
    param->lineno = 0;
    param->type = Integer;

    /* attach the parameter list in the convention your parser uses */
    /* I saw earlier you used child[0] for param list; keep that */
    outputFunc->child[0] = param;
    outputFunc->child[1] = NULL;
    outputFunc->attr.name = copyString("output");
    outputFunc->lineno = 0;
    outputFunc->type = Void;
    st_insert("output", 0, addLocation());
    b = getBucketList("output");
    if (b) { b->isFunc = 1; b->decl = outputFunc; }
}

/* Insert declarations into symbol table */
static void insertNode(TreeNode *t)
{
    if (!t) return;
    switch (t->nodekind)
    {
      case DeclK:
        if (Debug) fprintf(listing, "Declaration node at line %d\n", t->lineno);
        if (Debug) fprintf(listing, "Declaration kind - ");
        switch (t->kind.decl)
        {
            case FuncK:
                if (Debug) fprintf(listing, "Function declaration at line %d\n", t->lineno);
                if (Debug) fprintf(listing, "Function name: %s\n", t->attr.name);
                funcName = t->attr.name;
                if (st_lookup_top(funcName) == -1)
                {
                  /* insert function name in current scope */
                  st_insert(funcName, t->lineno, addLocation());
                  /* create a fresh scope for this function and attach it to the function node */
                  Scope newScope = createScope(funcName);
                  newScope->parent = currentScope();
                  pushScope(newScope);
                  /* attach pointer so we know we actually pushed a scope for this node */
                  t->attr.scope = currentScope();
                  BucketList b = getBucketList(funcName);
                  if (b) { b->isFunc = 1; b->decl = t; }
                  
                  // inFuncDecl = TRUE;
                }
                else
                  symbolredifineError(t, funcName);
                break;
            case VarK:
                if (Debug) fprintf(listing, "Variable declaration at line %d\n", t->lineno);
                if (Debug) fprintf(listing, "Variable name: %s\n", t->attr.name);
                if (Debug) fprintf(listing, "Variable data type: %s\n", t->type == Void ? "Void" : "Integer");

                if (t->type == Void) {
                  fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
                  Error = TRUE;
                }
                else {
                  funcName = t->attr.name;
                  if (st_lookup_top(t->attr.name) == -1) {
                      st_insert(funcName, t->lineno, addLocation());
                      /* record declaration node in bucket so type info is available later */
                      BucketList b = getBucketList(funcName);
                      if (b) b->decl = t;
                  }
                  else
                    symbolredifineError(t, funcName);
                }
                break;
            case ArrVarK:
                if (Debug) fprintf(listing, "Variable declaration at line %d\n", t->lineno);
                if (Debug) fprintf(listing, "Variable name: %s\n", t->attr.arr.name);
                if (Debug) fprintf(listing, "Variable data type: %s\n", t->type == Void ? "Void" : "Integer");

                if (t->type == Void) {
                  fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.arr.name);
                  Error = TRUE;
                }
                else {
                  funcName = t->attr.arr.name;
                  if (st_lookup_top(t->attr.arr.name) == -1) {
                      st_insert(funcName, t->lineno, addLocation());
                      /* record declaration node in bucket so type info is available later */
                      BucketList b = getBucketList(funcName);
                      if (b) b->decl = t;
                  }
                  else
                    symbolredifineError(t, funcName);
                }
                break;
        }
        break;
      case ParamK:
        if (Debug) fprintf(listing, "Parameter node at line %d\n", t->lineno);
        if (Debug) fprintf(listing, "Parameter kind: %d\n", t->kind.param);
        if (Debug) fprintf(listing, "Parameter name: %s\n", t->attr.name);

        if (t->type == Void && strcmp(t->attr.name, "void") != 0) {
          fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
          Error = TRUE;
        }
        else {
          funcName = t->attr.name;
          if (st_lookup_top(t->attr.name) == -1) {
              st_insert(t->attr.name, t->lineno, addLocation());
              if (t->kind.param == ArrParamK)
                if (Debug) fprintf(listing, "Parameter is an array\n");
              /* record declaration node in bucket so type info is available later */
              if (t->kind.param == NonArrParamK) {
                if (Debug) fprintf(listing, "Recording non-array parameter declaration for %s\n", t->attr.name);
                t->type = Integer;
              }
              BucketList b = getBucketList(funcName);
              if (b) b->decl = t;
          }
          else
            symbolredifineError(t, funcName);
        }
        break;
      case StmtK:
        if(t->kind.stmt == CompK) {
          if (Debug) fprintf(listing, "Compound statement at line %d %s\n", t->lineno, t->attr.scope ? t->attr.scope->funcName : "global");
          if(preserveLastScope) {
              preserveLastScope = FALSE;
              // t->attr.scope = NULL;
          }
          else {
              Scope newScope = createScope(funcName);
              newScope->parent = currentScope();  // 현재 Scope를 부모로 지정
              pushScope(newScope);
              t->attr.scope = currentScope();
          }
        }
        break;

      case ExpK:
        switch (t->kind.exp) {
          case ArrIdK:
          case IdK:
          case CallK:
            if (t->attr.name) funcName = t->attr.name;
            else funcName = t->attr.arr.name;

            if (st_lookup(funcName) == -1)
              /* not yet in table, error */
              fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", funcName, t->lineno);
            else
              st_addLineNumber(funcName, t->lineno);
            break;
        }
        break;
      default:
        break;
    }
}

/* Pop function scope after insertion */
static void afterInsertNode(TreeNode *t)
{
  if (!t) return;
    /* debug-safe print */
    // if (Debug) {
    //     const char *name = NULL;
    //     if (t->nodekind == DeclK) {
    //         if (t->kind.decl == ArrVarK) name = t->attr.arr.name;
    //         else name = t->attr.name;
    //     } else if (t->nodekind == ParamK || t->nodekind == ExpK) {
    //         name = t->attr.name;
    //     }
    //     fprintf(listing, "after insert node is here %s\n", name ? name : "(null)");
    // }

    /* Only pop if this node actually had a scope attached (i.e., we pushed for it) */
    if (t->nodekind == DeclK && t->kind.decl == FuncK) {
        if (t->attr.scope) {
            popScope();
            // t->attr.scope = NULL;
        }
    }
    if (t->nodekind == StmtK && t->kind.stmt == CompK) {
        if (t->attr.scope) {
            popScope();
            // t->attr.scope = NULL;
        }
    }
}

/* Build symbol table from syntax tree */
void buildSymtab(TreeNode *syntaxTree)
{
    if (Debug) fprintf(listing, "now traverse ... \n");
    globalscope = createScope(NULL);
    pushScope(globalscope);
    InsertIOFunctions();
    traverse(syntaxTree, insertNode, afterInsertNode);
    popScope();

    if (TraceAnalyze)
    {
        fprintf(listing, "\nSymbol table:\n\n");
        printSymTab(listing);
    }
}

/* Type checking */
static void checkNode(TreeNode *t)
{
    if (!t) return;
    switch (t->nodekind)
    {
      case ExpK:
        if (Debug) fprintf(listing, "Expression node at line %d\n", t->lineno);
        if (Debug) fprintf(listing, "Expression kind: Expk - ");
        switch (t->kind.exp)
        {
            case IdK:
            {
                funcName = t->attr.name;
                if (Debug) fprintf(listing, "Identifier at line %d, %s\n", t->lineno, t->attr.name);
                BucketList b = getBucketList(funcName);
                if (!b)
                {
                  fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", t->attr.name, t->lineno);
                  Error = TRUE;
                  t->type = Undetermined;
                }
                else
                    t->type = b->decl ? b->decl->type : Undetermined;
                break;
            }
            case ArrIdK:
            {
              if (Debug) fprintf(listing, "Array identifier at line %d\n", t->lineno);
              if (Debug) fprintf(listing, "Array name: %s\n", t->attr.name);
              BucketList b = getBucketList(t->attr.name);
              if (!b)
              {
                fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", t->attr.name, t->lineno);
                Error = TRUE;
                t->type = Undetermined;
              }
              else
                  t->type = b->decl ? b->decl->type : Undetermined;

              if (t->kind.exp == ArrIdK && t->child[0] && t->child[0]->type != Integer) {
                fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indicies should be integer\n", t->lineno, t->attr.name);
                Error = TRUE;
              }
              break;
            }
            case ConstK:
              if (Debug) fprintf(listing, "Constant node at line %d\n", t->lineno);

              t->type = Integer;
              break;
            case OpK:
              if (Debug) fprintf(listing, "Operator node at line %d\n", t->lineno);

              if (t->child[0] && t->child[1])
              {
                int leftline = t->child[0]->lineno;
                int rightline = t->child[1]->lineno;

                if (leftline != rightline) {
                  fprintf(listing, "Error: invalid operation at line %d\n", leftline);
                  Error = TRUE;
                  break;
                }

                else if (t->child[0]->type != Integer || t->child[1]->type != Integer) {
                  fprintf(listing, "Error: invalid operation at line %d\n", t->lineno);
                  Error = TRUE;
                }
                    
                else
                    t->type = Integer;
            }
              break;
            case AssignK:
              if (Debug) fprintf(listing, "Assignment node at line %d\n", t->lineno);
              if (t->child[0] && t->child[1])
              {
                int leftline = t->child[0]->lineno;
                int rightline = t->child[1]->lineno;

                if (leftline != rightline) {
                  fprintf(listing, "Error: invalid operation at line %d\n", leftline);
                  Error = TRUE;
                  break;
                }

                else if (t->child[0]->type != t->child[1]->type) {
                  fprintf(listing, "Error: invalid assignment at line %d\n", t->lineno);
                  Error = TRUE;
                  break;
                }

                t->type = t->child[0]->type;
              }
              break;
            case CallK:
            {
              funcName = t->attr.name;
              if (Debug) fprintf(listing, "Function call node at line %d\n", t->lineno);
              if (Debug) fprintf(listing, "Function name: %s\n", funcName);
              BucketList b = getBucketList(funcName);

              if (!b || !b->isFunc || !b->decl) {
                fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", funcName, t->lineno);
                Error = TRUE;
                t->type = Undetermined;
                break;
              }
              
              TreeNode * funcDecl = b->decl;
              TreeNode *arg;
              TreeNode *param;
              
              if (!funcDecl) {
                fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", funcName, t->lineno);
                Error = TRUE;
                t->type = Undetermined;
                break;
              }

              /* Check parameter count and argument types */
              arg = t->child[0];         // actual arguments
              param = funcDecl->child[0]; // formal parameters
              
              /* Walk both lists in parallel */
              while (arg != NULL && param != NULL) {
                if (arg->type == Void) {
                  fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, arg->attr.name);
                  Error = TRUE;
                  break;
                }

                // if (arg->type != param->type) {
                //   fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", lineno, funcName);
                //   Error = TRUE;
                // }

                arg = arg->sibling;
                param = param->sibling;
              }
              
              if (Error) break;

              /* If one list is longer than the other → mismatch */
              if (arg != NULL || param != NULL) {
                fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, funcName);
                Error = TRUE;
                break;
              }

              /* Return function type */
              t->type = funcDecl->type;
            }
            break;

          default:
            break;
        }
        break;
      case StmtK:
        if (Debug) fprintf(listing, "Statement node at line %d\n", t->lineno);
        if (Debug) fprintf(listing, "Statement kind: StmtK\n");
        switch (t->kind.stmt)
        {
          case IfK:
          case IterK:
            if (Debug) fprintf(listing, "Iteration/Condition node at line %d\n", t->lineno);

            int leftline = t->child[0]->lineno;
            int rightline = t->child[1]->lineno;

            if (leftline != rightline) {
              fprintf(listing, "Error: invalid condition at line %d\n", rightline);
              Error = TRUE;
            }

            if (!t->child[0] || t->child[0]->type != Integer) {
              fprintf(listing, "Error: invalid condition at line %d\n", rightline);
              Error = TRUE;
            }
            break;
          case RetK:
            if (Debug) fprintf(listing, "Return statement node at line %d\n", t->lineno);
            Scope s = currentScope();
            while (s && !s->funcName) s = s->parent;
            if (s)
            {
              BucketList b = getBucketList(s->funcName);
              if (b && b->decl)
              {
                ExpType expected = b->decl->type;
                ExpType actual = t->child[0] ? t->child[0]->type : Void;
                if (expected != actual) {
                    fprintf(listing, "Error: Invalid return at line %d\n", t->lineno);
                    Error = TRUE;
                }
              }
            }
            break;
          case CompK:
            if (Debug) fprintf(listing, "Compound statement at line %d\n", t->lineno);
            popScope();
            break;
          default:
            break;
        }
        break;
    default:
      break;
  }
}

// static void preTypeProc(TreeNode *t)
// {
//     // fprintf(listing, "Pre-order traversal at line %d\n", t->lineno);
//     if (!t) return;
//     if (t->nodekind == DeclK && t->kind.decl == FuncK && t->attr.scope)
//       pushScope(t->attr.scope);
//     if( t->nodekind == StmtK && t->kind.stmt == CompK && t->attr.scope)
//       pushScope(t->attr.scope);
// }

// static void postTypeProc(TreeNode *t)
// {
//     // fprintf(listing, "Post-order traversal at line %d\n", t->lineno);
//     if (!t) return;
//     checkNode(t);
//     if (t->nodekind == DeclK && t->kind.decl == FuncK)
//         popScope();
//     if (t->nodekind == StmtK && t->kind.stmt == CompK)
//         popScope();
// }

static void beforeChecknode(TreeNode *t)
{
  if(!t) return;
  if (t->nodekind == DeclK && t->kind.decl == FuncK && t->attr.scope) {
    pushScope(t->attr.scope);
  }
  // CompoundStmt(블록)일 때만 push
  if (t->nodekind == StmtK && t->kind.stmt == CompK && t->attr.scope) {
    // if (inFuncDecl) {
    //   // CompK는 함수 body임
    //   // pushScope는 이미 함수 선언에서 했으므로 하지 않음
    //   inFuncDecl = FALSE;   // body 처리 끝
    // }
    pushScope(t->attr.scope);
  }
}



/* Type check traversal */
void typeCheck(TreeNode *syntaxTree)
{
    /* start with global scope on stack */
    pushScope(globalscope);
    traverse(syntaxTree, beforeChecknode, checkNode);
    popScope();
}

