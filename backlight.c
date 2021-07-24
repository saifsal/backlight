#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define BRIGHTNESS "/sys/class/backlight/intel_backlight/brightness"
#define MAX_BRIGHTNESS "/sys/class/backlight/intel_backlight/max_brightness"

#define BUFFER_SIZE 8

long read_file(char const *const filename) {
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    perror(filename);
    return -1;
  }

  char buffer[BUFFER_SIZE];

  if (fgets(buffer, BUFFER_SIZE, file) == NULL) {
    fclose(file);
    fprintf(stderr, "Failed to read %s\n", filename);
    return -1;
  }

  fclose(file);

  char *end;
  errno = 0;

  long value = strtol(buffer, &end, 10);

  if (errno != 0 || end == buffer || value < 0) {
    fprintf(stderr, "Invalid value in %s\n", filename);
    return -1;
  }

  return value;
}

long read_percentage(char const *const string) {
  char *end;
  errno = 0;

  long value = strtol(string, &end, 10);

  if (errno != 0 || end == string || *end != '\0' ||
      value < 0 || value > 100) {
    fprintf(stderr, "Percentage must be an integer between 0 and 100\n");
    return -1;
  }

  return value;
}

int write_file(char const *const filename, long const brightness) {
  FILE *file = fopen(filename, "w");

  if (file == NULL) {
    perror(filename);
    return -1;
  }

  if (fprintf(file, "%ld", brightness) < 0) {
    perror(filename);
    fclose(file);
    return -1;
  }

  fclose(file);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s <i|d|s|g> [percentage]\n", argv[0]);
    return 1;
  }

  long brightness = read_file(BRIGHTNESS);
  long const maximum = read_file(MAX_BRIGHTNESS);

  if (brightness < 0 || maximum < 0)
    return 1;

  long const minimum = 0;

  char const command = argv[1][0];

  long const percentage = argc == 3 ? read_percentage(argv[2]) : 1;

  if (percentage < 0)
    return 1;

  long const amount = maximum * percentage / 100l;

  if (command == 'i') {
    brightness += amount;
  } else if (command == 'd') {
    brightness -= amount;
  } else if (command == 's') {
    brightness = amount;
  } else if (command == 'g') {
    fprintf(stdout, "Backlight: %.2f\n", (double)(brightness) / maximum);
  } else {
    fprintf(stderr, "Invalid command: %c\n", command);
    return 1;
  }

  if (brightness > maximum) {
    brightness = maximum;
  } else if (brightness < minimum) {
    brightness = minimum;
  }

  return write_file(BRIGHTNESS, brightness);
}
