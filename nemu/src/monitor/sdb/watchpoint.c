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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  word_t val;
  char *expr;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp() {
  if (free_ == NULL) {
    panic("no more watchpoint");
  }
  WP *wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  return wp;
}
void free_wp(WP *wp) {
  if (wp == head) {
    head = head->next;
  } else {
    WP *p = head;
    while (p->next != wp) {
      p = p->next;
    }
    p->next = wp->next;
  }
  wp->next = free_;
  free_ = wp;
}

int set_watchpoint(char *e) {
  bool success;
  word_t val = expr(e, &success);
  if (!success) return -1;

  WP *p = new_wp();
  p->expr = strdup(e);
  p->val = val;

  return p->NO;
}

bool delete_watchpoint(int NO) {
  WP *p;
  for (p = head; p != NULL; p = p->next) {
    if (p->NO == NO) { break; }
  }

  if (p == NULL) { return false; }

  free_wp(p);
  return true;
}

void list_watchpoint() {
  if (head == NULL) {
    printf("No watchpoints\n");
    return;
  }

  printf("%8s\t%8s\t%8s\n", "NO", "Address", "Enable");
  WP *p;
  for (p = head; p != NULL; p = p->next) {
    printf("%8d\t%s\t" FMT_WORD "\n", p->NO, p->expr, p->val);
  }
}

void scan_watchpoint() {
  for (WP *p = head; p; p = p->next) {
    bool success = true;
    word_t val = expr(p->expr, &success);
    if (!success) {
      printf("Expression %s is invalid", p->expr);
      continue ;
    }
    if (val != p->val) {
      printf("Expression %s changed from %u to %u", p->expr, p->val, val);
      p->val = val;
      nemu_state.state = NEMU_STOP;
    }
  }
}

/* TODO: Implement the functionality of watchpoint */

