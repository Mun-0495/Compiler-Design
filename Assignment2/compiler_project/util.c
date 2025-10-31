/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken( TokenType token, const char* tokenString )
{ switch (token)
  { case ELSE:
    case IF:
    case INT:
    case RETURN:
    case VOID:
    case WHILE:
      fprintf(listing,
         "reserved word: %s\n",tokenString);
      break;
    case PLUS: fprintf(listing,"+\n"); break;
    case MINUS: fprintf(listing,"-\n"); break;
    case TIMES: fprintf(listing,"*\n"); break;
    case OVER: fprintf(listing,"/\n"); break;
    case LT: fprintf(listing,"<\n"); break;
    case LE: fprintf(listing,"<=\n"); break;
    case GT: fprintf(listing,">\n"); break;
    case GE: fprintf(listing,">=\n"); break;
    case EQ: fprintf(listing,"==\n"); break;
    case NE: fprintf(listing,"!=\n"); break;
    case ASSIGN: fprintf(listing,"=\n"); break;
    case SEMI: fprintf(listing,";\n"); break;
    case COMMA: fprintf(listing,",\n"); break;
    case LPAREN: fprintf(listing,"(\n"); break;
    case RPAREN: fprintf(listing,")\n"); break;
    case LBRACE: fprintf(listing,"[\n"); break;
    case RBRACE: fprintf(listing,"]\n"); break;
    case LCURLY: fprintf(listing,"{\n"); break;
    case RCURLY: fprintf(listing,"}\n"); break;
    case ENDFILE: fprintf(listing,"EOF\n"); break;
    case NUM:
      fprintf(listing,
          "NUM, val= %s\n",tokenString);
      break;
    case ID:
      fprintf(listing,
          "ID, name= %s\n",tokenString);
      break;
    case ERROR:
      fprintf(listing,
          "ERROR: %s\n",tokenString);
      break;
    default: /* should never happen */
      fprintf(listing,"Unknown token: %d\n",token);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function newParamNode creates a new declation
 * node for syntax tree construction
 */
TreeNode * newDeclNode(DeclKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newParamNode creates a new parameter
 * node for syntax tree construction
 */
TreeNode * newParamNode(ParamKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ParamK;
    t->kind.param = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */

const char* TreeNodeType[] = {
    "StmtK",
    "ExpK",
    "DeclK",
    "ParamK"
};
void printTree( TreeNode * tree )
{ 
  int i;
  INDENT;
  while(tree != NULL) {
      printSpaces();
      // printf("TreeNode Type: %s\n", TreeNodeType[tree->nodekind]);
      // printf("TreeNode LineNo: %d\n", tree->lineno);
      switch(tree->nodekind) {
        case StmtK:
          switch(tree->kind.stmt) {
              case CompK: fprintf(listing,"Compound Statement:\n"); break;
              case IfK: fprintf(listing,"If-Else Statement:\n"); break;
              case IterK: fprintf(listing,"While Statement:\n"); break;
              case RetK: fprintf(listing,"Return Statement:\n"); break;
              default: fprintf(listing,"Unknown StmtNode kind\n"); break;
            }
          break;
        case ExpK:
          switch(tree->kind.exp) {
            case AssignK: fprintf(listing,"Assign:\n"); break;
            case OpK: fprintf(listing,"Op: "); printToken(tree->attr.op,"\0"); break;
            case ConstK: fprintf(listing,"Const: %d\n",tree->attr.val); break;
            case IdK: fprintf(listing,"Variable: name = %s\n",tree->attr.name); break;
            case ArrIdK: fprintf(listing,"Array Variable: name = %s\n",tree->attr.name); break;
            case CallK:
                fprintf(listing,"Call: function name = %s\n",tree->attr.name);
                break;
            default: fprintf(listing,"Unknown ExpNode kind\n"); break;
          }
          break;
        case DeclK:
          switch(tree->kind.decl) {
              case FuncK:
                  fprintf(listing,"Function Declaration: name = %s, return type = %s\n",
                          tree->attr.name,
                          tree->type==Integer?"int":"void");
                  break;
              case VarK:
                  fprintf(listing,"Variable Declaration: name = %s, type = %s\n",
                          tree->attr.name,
                          tree->type==Integer?"int":"void");
                  break;
              case ArrVarK:
                  fprintf(listing,"Array Variable Declaration: name = %s, type = %s, size = %d\n",
                          tree->attr.arr.name,
                          tree->type==IntegerArray?"int[]":"void",
                          tree->attr.arr.size);
                  break;
              default: fprintf(listing,"Unknown DeclNode kind\n"); break;
          }
          break;
        case ParamK:
          switch(tree->kind.param) {
              case NonArrParamK:
                if (tree->type == Void) {
                    fprintf(listing,"Void Parameter\n");
                    break;
                }
                fprintf(listing,"Parameter: name = %s, type = %s\n",
                        tree->attr.name,
                        tree->type==Integer?"int":"void");
                break;
              case ArrParamK:
                if (tree->type == Void) {
                    fprintf(listing,"Void Parameter\n");
                    break;
                }
                fprintf(listing,"Array Parameter: name = %s, type = %s\n",
                        tree->attr.name,
                        tree->type==IntegerArray?"int[]":"void");
                break;
              default: fprintf(listing,"Unknown ParamNode kind\n"); break;
          }
          break;
        default:
          fprintf(listing,"Unknown node kind\n");
      }

      for(i=0;i<MAXCHILDREN;i++) {
          printTree(tree->child[i]);
      }

      tree = tree->sibling;
  }
  UNINDENT;
}