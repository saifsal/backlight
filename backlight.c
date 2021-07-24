#include <stdio.h>
#include <stdlib.h>

#define BRIGHTNESS "/sys/class/backlight/intel_backlight/brightness"
#define MAX "/sys/class/backlight/intel_backlight/max_brightness"

#define SIZE 8

int read_file(char const *const filename) {
  FILE *fp = fopen(filename, "r");

  if (fp == NULL)
    return 0;

  char buf[SIZE];
  fgets(buf, SIZE, fp);
  fclose(fp);
  return atoi(buf);
}

void write_file(char const *const filename, int const bright) {
  FILE *fp = fopen(filename, "w");

  if (fp == NULL)
    return;

  fprintf(fp, "%i", bright);
  fclose(fp);
}

int main(int argc, char *argv[]) {
  if (argc > 3 || argc < 2) {
    return 0;
  }

  int bright = read_file(BRIGHTNESS);

  int const max = read_file(MAX);

  char const c = argv[1][0];

  int v = 1;

  if (argc == 3) {
    v = atoi(argv[2]);
  }

  if (c == 'i') {
    bright += v;
  } else if (c == 'd') {
    bright -= v;
  } else if (c == 's') {
    bright = v;
  } else {
    return 0;
  }

  if (bright > max) {
    bright = max;
  } else if (bright < 0) {
    bright = 0;
  }

  write_file(BRIGHTNESS, bright);

  return 0;
}
