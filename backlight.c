#include <stdio.h>
#include <stdlib.h>

#define BRIGHTNESS "/sys/class/backlight/intel_backlight/brightness"
#define MAX_BRIGHTNESS "/sys/class/backlight/intel_backlight/max_brightness"

#define SIZE 8

int read_file(char const *const filename) {
  FILE *file = fopen(filename, "r");

  if (file == NULL)
    return 0;

  char buffer[SIZE];
  fgets(buffer, SIZE, file);
  fclose(file);
  return atoi(buffer);
}

void write_file(char const *const filename, int const brightness) {
  FILE *file = fopen(filename, "w");

  if (file == NULL)
    return;

  fprintf(file, "%i", brightness);
  fclose(file);
}

int main(int argc, char *argv[]) {
  if (argc > 3 || argc < 2) {
    return 0;
  }

  int brightness = read_file(BRIGHTNESS);

  int const maximum = read_file(MAX_BRIGHTNESS);
  int const minimum = 0;

  char const command = argv[1][0];

  int const volume = argc == 3 ? atoi(argv[2]) : 1;

  if (command == 'i') {
    brightness += volume;
  } else if (command == 'd') {
    brightness -= volume;
  } else if (command == 's') {
    brightness = volume;
  } else {
    return 0;
  }

  if (brightness > maximum) {
    brightness = maximum;
  } else if (brightness < minimum) {
    brightness = minimum;
  }

  write_file(BRIGHTNESS, brightness);

  return 0;
}
