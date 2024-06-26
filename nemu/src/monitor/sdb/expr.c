/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "memory/vaddr.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM,
  TK_NEQ, TK_OR, TK_AND, TK_REG, TK_REF, TK_NEG
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal

  {"0x[0-9a-fA-F]{1,16}", TK_NUM},  // hex
  {"[0-9]{1,10}", TK_NUM},         // dec
  {"\\$[a-z0-9]{1,31}", TK_REG},   // register names
  {"\\+", '+'},
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"%", '%'},
  {"==", TK_EQ},
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!", '!'},
  {"\\(", '('},
  {"\\)", ')'}

  //{"^(\\-|\\+)?\\d+(\\.\\d+)?$", TK_NUM},         // plus
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE : 
            break ;
          case TK_NUM:
          case TK_REG: sprintf(tokens[nr_token].str, "%.*s", substr_len, substr_start);
          default: tokens[nr_token].type = rules[i].token_type;
                   nr_token ++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


// static int check_parentheses(int p, int q) {
//   int ind = 0;
//   for (int i = p; i <= q; i++) {
//     if (tokens[i].type == '(') {
//       ind++;
//     } else if (tokens[i].type == ')') {
//       if (ind == 0) return false;
//       ind --;
//     }
//   }
//   return ind == 0 ? true : false;
// }

static int op_prec(int t) {
  switch (t) {
    case '!': case TK_NEG: case TK_REF: return 0;
    case '*': case '/': case '%': return 1;
    case '+': case '-': return 2;
    case TK_EQ: case TK_NEQ: return 4;
    case TK_AND: return 8;
    case TK_OR: return 9;
    default: assert(0);
  }
}

static inline int op_prec_cmp(int t1, int t2) {
  return op_prec(t1) - op_prec(t2);
}

static int find_dominated_op(int s, int e, bool *success) {
  int i;
  int bracket_level = 0;
  int dominated_op = -1;
  for (i = s; i <= e; i ++) {
    switch (tokens[i].type) {
      case TK_REG: case TK_NUM: break;

      case '(':
        bracket_level ++;
        break;

      case ')':
        bracket_level --;
        if (bracket_level < 0) {
          *success = false;
          return 0;
        }
        break;

      default:
        if (bracket_level == 0) {
          if (dominated_op == -1 ||
              op_prec_cmp(tokens[dominated_op].type, tokens[i].type) < 0 ||
              (op_prec_cmp(tokens[dominated_op].type, tokens[i].type) == 0 &&
               tokens[i].type != '!' && tokens[i].type != '~' &&
               tokens[i].type != TK_NEG && tokens[i].type != TK_REF) ) {
            dominated_op = i;
          }
        }
        break;
    }
  }

  *success = (dominated_op != -1);
  return dominated_op;
}


// static int findMainOp(int p, int q) {
//   int ind = 0;
//   int ret = -1;
//   for (int i = p; i <= q; i++)
//   {
//     if (tokens[i].type == '(')
//     {
//       ind++;
//       continue;
//     }
//     else if (tokens[i].type == ')')
//     {
//       ind--;
//       continue;
//     }
//     if (tokens[i].type == TK_NUM || ind != 0) {
//       continue ;
//     }
//     switch (tokens[i].type)
//     {
//     case '+':
//     case '-': {
//       ret = i;
//     } break ;

//     case '*':
//     case '/': {
//       if (ret == -1 || tokens[ret].type == '*' || tokens[ret].type == '/') {
//         ret = i;
//       }
//     } break;

//     default:
//       break;
//     }
//   }
//   return ret;
// }

word_t eval(int p, int q, bool *success) {
  if (p > q) return 0;
  else if (p == q) {
    if (tokens[p].type == TK_NUM) {
      return strtoul(tokens[p].str, NULL, 0);
    } else if (tokens[p].type == TK_REG) {
      return isa_reg_str2val(tokens[p].str, success);
    } else {
      *success = false;
    }
    
  } else if (tokens[p].type == '(' && tokens[q].type == ')') {
    return eval(p + 1, q - 1, success);
  } else {
    int mop = find_dominated_op(p, q, success);
    
    int op_type = tokens[mop].type;
    if (op_type == '!' || op_type == TK_NEG || op_type == TK_REF) {
      word_t val = eval(mop + 1, q, success);
      if (!*success) { return 0; }

      switch (op_type) {
        case '!': return !val;
        case TK_NEG: return -val;
        case TK_REF: return vaddr_read(val, 4);
        default: assert(0);
      }
    }

    if (!*success) { return 0; }
    word_t val1 = eval(p, mop - 1, success);
    if (!*success) { return 0; }
    word_t val2 = eval(mop + 1, q, success);
    if (!*success) { return 0; }

    switch (tokens[mop].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case '%': return val1 % val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      case TK_OR: return val1 || val2;
      
      default: assert(0);
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')'))) {
      tokens[i].type = TK_REF;
    }
  }
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')'))) {
      tokens[i].type = TK_NEG;
    }
  }
  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token - 1, success);

}
