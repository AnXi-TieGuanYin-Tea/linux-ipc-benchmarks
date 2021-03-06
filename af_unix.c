#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "decls.h"

typedef struct state { int pair[2]; } state;

state *new_state() { return calloc(sizeof(state), 1); }

void free_state(state *state) { free(state); }

int pre_fork_setup(state *state) {
  if (socketpair(AF_UNIX, SOCK_STREAM, 0 /* protocol */, state->pair)) {
    // uh-oh
  }
  return 0;
}

int cleanup(state *state __attribute__((unused))) { return 0; }

int child_post_fork_setup(state *state __attribute__((unused))) { return 0; }

int child_warmup(int warmup_iters __attribute__((unused)),
                 state *state __attribute__((unused))) {
  return 0;
}

int child_loop(int iters __attribute__((unused)), state *state) {
  int tx_fd = state->pair[1];
  int rx_fd = state->pair[0];

  for (;;) {
    char msg;
    read(rx_fd, &msg, 1);
    write(tx_fd, "1", 1);
  }
  return 0;
}

int child_cleanup(state *state) {
  close(state->pair[0]);
  close(state->pair[1]);

  return 0;
}

int parent_post_fork_setup(state *state __attribute__((unused))) { return 0; }

int parent_warmup(int warmup_iters, state *state) {
  int tx_fd = state->pair[0];
  int rx_fd = state->pair[1];

  for (int i = 0; i < warmup_iters; ++i) {
    char resp;
    if (write(tx_fd, "0", 1) == -1) {
      perror("could not write");
      break;
    };

    if (read(rx_fd, &resp, 1) == -1) {
      perror("could not read");
      break;
    }
  }
  return 0;
}

int parent_loop(int iters, state *state) {
  int tx_fd = state->pair[0];
  int rx_fd = state->pair[1];

  for (int i = 0; i < iters; ++i) {
    char resp;
    if (write(tx_fd, "0", 1) == -1) {
      perror("could not write");
      break;
    };

    if (read(rx_fd, &resp, 1) == -1) {
      perror("could not read");
      break;
    }
  }
  return 0;
}

int parent_cleanup(state *state) {
  close(state->pair[0]);
  close(state->pair[1]);

  return 0;
}
