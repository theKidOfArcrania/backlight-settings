#include <string>
#include <iostream>
#include <fstream>
#include <tclap/CmdLine.h>

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

struct argdata {
  char setting;
  char relative;
  int perc;
};

class PercentageConstraint : public TCLAP::Constraint<int> {
public:
  virtual std::string description() const {
    return "percentage (0 - 100%)";
  }

  virtual std::string shortID() const {
    return "percentage";
  }

  virtual bool check(const int& value) const {
    return value >= 0 && value <= 100;
  }
};

#define PATH "/sys/class/backlight/intel_backlight"

// Function prototypes
int main(int argc, char** argv);
void processArgs(const argdata& params);
argdata parseArgs(int argc, char** argv);
// End function prototypes

int main(int argc, char** argv) {
  setregid(getegid(), getegid()); // We are a setgid binary.

  argdata params = parseArgs(argc, argv);

  // Check we have intel_backlight
  struct stat buf;
  if (stat(PATH, &buf)) {
    std::cerr << "Your system does not have intel_backlight. Cannot configure "
      "backlight brightness!" << std::endl;
    exit(1);
  }

  // Check that we have write access
  if (params.setting && access(PATH "/brightness", W_OK | R_OK)) {
    std::cerr << "Cannot access \"" PATH "/brightness\": " << strerror(errno) 
      << std::endl;
    exit(1);
  }

  processArgs(params);
}

void processArgs(const argdata& params) {
  int maxBrightness = -1;
  int brightness = -1;

  // Obtain max brightness
  std::ifstream in(PATH "/max_brightness");
  if (in.fail()) {
    std::cerr << "Unable to get max brightness.";
    exit(1);
  }

  in >> maxBrightness;
  if (in.fail() || maxBrightness == -1) {
    std::cerr << "Unable to get max brightness.";
    exit(1);
  }
  in.close();

  // Obtain current brightness
  in.clear();
  in.open(PATH "/brightness");
  if (in.fail()) {
    std::cerr << "Unable to get current brightness.";
    exit(1);
  }

  in >> brightness;
  if (in.fail() || brightness == -1) {
    std::cerr << "Unable to get current brightness.";
    exit(1);
  }
  in.close();

  if (params.setting) {
    // Setting new brightness
    double unit = maxBrightness * .01;
    if (params.relative)
      brightness = (int)std::round(brightness + params.perc * unit);
    else
      brightness = (int)std::round(params.perc * unit);
    brightness = std::max(std::min(brightness, maxBrightness), 0);

    std::ofstream out(PATH "/brightness");
    if (out.fail()) {
      std::cerr << "Unable to set current brightness.";
      exit(1);
    }

    out << brightness;
    if (out.fail()) {
      std::cerr << "Unable to set current brightness.";
      exit(1);
    }

    out.close();
  } else {
    // Print current brightness
    printf("Brightness: %d/%d %.2f%%\n", brightness, maxBrightness, brightness 
        * 100.0 / maxBrightness);
  }
}

argdata parseArgs(int argc, char** argv) {
  TCLAP::CmdLine cmd("Backlight Settings v0.9\nYet another program that adjusts "
      "a laptop screen backlight.", ' ', "0.9");

  // Add valid arguments and flags
  PercentageConstraint pct;
  TCLAP::ValueArg<int> inc("", "inc", "Increments backlight brightness by a "
      "percentage of max brightness.", false, -1, &pct, cmd);
  TCLAP::ValueArg<int> dec("", "dec", "Decrements backlight brightness by a "
      "percentage of max brightness.", false, -1, &pct, cmd);
  TCLAP::ValueArg<int> set("", "set", "Sets the backlight brightness to a "
      "percentage of max brightness.", false, -1, &pct, cmd);
  TCLAP::SwitchArg get("", "get", "Obtains the current backlight brightness.", cmd);

  // Parse arguments
  cmd.parse(argc, argv);

#define declHas(var) int __has_ ## var = var.getValue() != -1
#define has(var) __has_ ## var

  declHas(inc);
  declHas(dec);
  declHas(set);

  if (has(inc) + has(dec) + has(set) + get.getValue() > 1) {
    std::cerr << "Must have exactly one of the following flags: inc, dec, "
      "set, get." << std::endl;
    exit(1);
  }

  argdata res;
  res.setting = true;
  if (has(inc)) {
    res.relative = true;
    res.perc = inc.getValue();
  } else if (has(dec)) {
    res.relative = true;
    res.perc = -dec.getValue();
  } else if (has(set)) {
    res.relative = false;
    res.perc = set.getValue();
  } else if (get.getValue()){ 
    res.setting = false;
  } else {
    cmd.getOutput()->usage(cmd);
    exit(2);
  }
  
  return res;
}
