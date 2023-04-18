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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM,

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

  {"\\-", '-'},         // plus
  {"\\*", '*'},         // plus
  {"\\/", '/'},         // plus
  {"\\(", '('},         // plus
  {"\\)", ')'},         // plus

  {"^(\\-|\\+)?\\d+(\\.\\d+)?$", TK_NUM},         // plus
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
          case TK_NOTYPE : {

          } break ;
          case '+' : {
            tokens[nr_token++].type = '+';
          } break ;
          case '-' : {
            tokens[nr_token++].type = '-';
          } break ;
          case '*' : {
            tokens[nr_token++].type = '*';
          } break ;
          case '/' : {
            tokens[nr_token++].type = '/';
          } break ;
          case TK_EQ : {
            tokens[nr_token++].type = TK_EQ;
          } break ;
          case TK_NUM : {
            tokens[nr_token].type = TK_NUM;
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
          } break ;
          default: TODO();
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


static int check_parentheses(int p, int q) {
  int ind = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      ind++;
    } else if (tokens[i].type == ')') {
      if (ind == 0) return false;
      ind --;
    }
  }
  return ind == 0 ? true : false;
}

static int findMainOp(int p, int q) {
  int ind = 0;
  int ret = -1;
  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == '(')
    {
      ind++;
      continue;
    }
    else if (tokens[i].type == ')')
    {
      ind--;
      continue;
    }
    if (tokens[i].type == TK_NUM || ind != 0) {
      continue ;
    }
    switch (tokens[i].type)
    {
    case '+':
    case '-': {
      ret = i;
    } break ;

    case '*':
    case '/': {
      if (ret == -1 || tokens[ret].type == '*' || tokens[ret].type == '/') {
        ret = i;
      }
    } break;

    default:
      break;
    }
  }
  return ret;
}

word_t eval(int p, int q) {
  if (p > q) return 0;
  else if (p == q) {
    if (tokens[p].type != TK_NUM) {
      return 0;
    }
    return atoi(tokens[p].str);
  } else if (tokens[p].type == '(' && tokens[p].type == ')' && check_parentheses(p + 1, q - 1) == true) {
    return eval(p + 1, q - 1);
  } else {
    int mop = findMainOp(p, q);
    assert(mop != -1);
    word_t val1 = eval(p, mop - 1);
    word_t val2 = eval(mop + 1, q);

    switch (tokens[mop].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e) || check_parentheses(0, nr_token - 1) == false) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token - 1);
  
  
  TODO();

  return 0;
}
