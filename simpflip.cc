#include <clocale>
#include <ctime>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

#define THREAD_COUNT 1
#define BUFFER_SIZE 100
// ONLY Mutate n% of the bytes in a buffer per run
#define MUTATION_COUNT 0.25 * BUFFER_SIZE

uint64_t iterations_count = 0;
std::time_t start = std::time(0);
std::time_t current;

void hexdump(void *ptr, int buflen) {
  unsigned char *buf = (unsigned char *)ptr;
  int i, j;
  for (i = 0; i < buflen; i += 16) {
    printf("%06x: ", i);
    for (j = 0; j < 16; j++)
      if (i + j < buflen)
        printf("%02x ", buf[i + j]);
      else
        printf("   ");
    printf(" ");
    for (j = 0; j < 16; j++)
      if (i + j < buflen)
        printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
    printf("\n");
  }
}

typedef struct buffers_t {
  char *data;
} Buffer;

void display() {
  std::time_t now = std::time(0);
  std::time_t elapsed = now - start;
  float cases_per_second = (float)iterations_count / (float)elapsed;
  printf("Elapsed %'9.2f ::: Cases %'lu ::: FuzzCasesPerSecond %'9.2f\n",
         (float)elapsed, iterations_count, cases_per_second);
}

void fuzz(uint32_t worker_id, Buffer buffer) {
  fprintf(stderr, "Worker %d Start\n", worker_id);
  char cloned[BUFFER_SIZE];
  for (;;) {
    int rand_mut = rand() % ((int)(MUTATION_COUNT + 0.5)) + 1;
    int rand_sleep = rand() % 30 + 1;
    // fprintf(stderr, "Buffer Start\n");
    // hexdump(buffer.data, BUFFER_SIZE);
    // fprintf(stderr, "%s\n", buffer.data);
    // fprintf(stderr, "Mutation Count %d\n", rand_mut);
    //  copy buffer to main original buffer ( dont want to constantly mutate it
    //  want to keep original style incase its a struct or something
    memcpy(cloned, buffer.data, BUFFER_SIZE);
    // Mutate Buffer By Flipping Bits Multiple Times
    for (size_t i = 0; i < rand_mut; i++) {
      int rand_bit = rand() % 8 + 0;
      int rand_byte = rand() % BUFFER_SIZE + 0;
      // fprintf(stderr, "Flipping Bit %d At Index %d\n", rand_bit, rand_byte);
      cloned[rand_byte] ^= (1 << rand_bit);
    }
    // send mutated buffer somewhere etc
    // ....
    // ....
    iterations_count++;
    display();
    // hexdump(cloned, BUFFER_SIZE);
    //  fprintf(stderr, "Iterations %lu\n", iterations_count);
    // sleep(rand_sleep);
  }
  fprintf(stderr, "Worker %d End\n", worker_id);
}

int main(int argc, char **argv) {
  setlocale(LC_NUMERIC, "");
  srand(0x7171);
  Buffer buffers[THREAD_COUNT];
  std::thread threads[THREAD_COUNT];
  for (uint32_t i = 0; i < THREAD_COUNT; i++) {
    // initalize buffers with random values
    buffers[i] = {(char *)calloc(1, BUFFER_SIZE)};
    for (size_t x = 0; x < BUFFER_SIZE; x++) {
      buffers[i].data[x] = rand() % 256;
    }
    // initalize threads pass buffers
    threads[i] = std::thread(fuzz, i + 1, buffers[i]);
  }
  // wait forever
  for (uint32_t i = 0; i < THREAD_COUNT; i++) {
    threads[i].join();
  }
  return 0;
}
