#include "platform-headless.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ares::Nintendo64 {
  auto option(string name, string value) -> bool;
  auto load(Node::System& node, string name) -> bool;
}

struct Options {
  string romPath;
  string dumpPath = "frame.ppm";

  u64 frames = 120;

  bool gpu = true;
  bool homebrew = true;
  bool recompiler = true;
  bool expansionPak = true;

  string controllerPakBanks = "32KiB (Default)";
};

static auto usage(const char* argv0) -> void {
  print(
    "usage:\n"
    "  ", argv0, " --rom <rom.z64> [options]\n"
    "  ", argv0, " <rom.z64> [frames] [dump.ppm]\n\n"

    "options:\n"
    "  --rom <path>              ROM path\n"
    "  --frames <n>              number of video frames to run, default 120\n"
    "  --dump <path>             output PPM path, default frame.ppm\n"
    "  --gpu <0|1>               enable Ares N64 GPU acceleration, default 1\n"
    "  --homebrew <0|1>          enable homebrew mode, default 1\n"
    "  --recompiler <0|1>        enable CPU recompiler, default 1\n"
    "  --expansion-pak <0|1>     enable Expansion Pak, default 1\n"
    "  --controller-pak-banks <value>\n"
    "                            default \"32KiB (Default)\"\n"
    "  --help                    show this help\n\n"

    "examples:\n"
    "  ", argv0, " --rom roms/cathode_quest.z64 --frames 1800 --dump cathode.ppm --gpu 1 --homebrew 1\n"
    "  ", argv0, " --rom roms/mario64.z64 --frames 1800 --dump mario.ppm --gpu 1 --homebrew 0\n"
  );
}

static auto parseBool(const char* s, bool fallback) -> bool {
  if(!s) return fallback;

  if(!strcmp(s, "1")) return true;
  if(!strcmp(s, "true")) return true;
  if(!strcmp(s, "on")) return true;
  if(!strcmp(s, "yes")) return true;

  if(!strcmp(s, "0")) return false;
  if(!strcmp(s, "false")) return false;
  if(!strcmp(s, "off")) return false;
  if(!strcmp(s, "no")) return false;

  return fallback;
}

static auto requireValue(int& i, int argc, char** argv, const char* flag) -> const char* {
  if(i + 1 >= argc) {
    print("missing value for ", flag, "\n");
    return nullptr;
  }

  return argv[++i];
}

static auto parseArgs(int argc, char** argv, Options& options) -> bool {
  if(argc < 2) return false;

  // Backward-compatible positional form:
  //   n64-headless rom.z64 [frames] [dump.ppm]
  if(argv[1][0] != '-') {
    options.romPath = argv[1];

    if(argc >= 3) options.frames = strtoull(argv[2], nullptr, 10);
    if(argc >= 4) options.dumpPath = argv[3];

    return true;
  }

  for(int i = 1; i < argc; i++) {
    const char* arg = argv[i];

    if(!strcmp(arg, "--help") || !strcmp(arg, "-h")) {
      return false;
    }

    if(!strcmp(arg, "--rom")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.romPath = value;
      continue;
    }

    if(!strcmp(arg, "--frames")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.frames = strtoull(value, nullptr, 10);
      continue;
    }

    if(!strcmp(arg, "--dump")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.dumpPath = value;
      continue;
    }

    if(!strcmp(arg, "--gpu")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.gpu = parseBool(value, options.gpu);
      continue;
    }

    if(!strcmp(arg, "--homebrew")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.homebrew = parseBool(value, options.homebrew);
      continue;
    }

    if(!strcmp(arg, "--recompiler")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.recompiler = parseBool(value, options.recompiler);
      continue;
    }

    if(!strcmp(arg, "--expansion-pak")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.expansionPak = parseBool(value, options.expansionPak);
      continue;
    }

    if(!strcmp(arg, "--controller-pak-banks")) {
      const char* value = requireValue(i, argc, argv, arg);
      if(!value) return false;
      options.controllerPakBanks = value;
      continue;
    }

    print("unknown argument: ", arg, "\n");
    return false;
  }

  return options.romPath;
}

static auto dumpPPM(const string& path, const std::vector<u32>& pixels, u32 width, u32 height) -> bool {
  if(!width || !height || pixels.empty()) return false;

  FILE* fp = fopen((const char*)path, "wb");
  if(!fp) return false;

  fprintf(fp, "P6\n%u %u\n255\n", width, height);

  for(auto pixel : pixels) {
    u8 rgb[3] = {
      u8(pixel >> 16),
      u8(pixel >> 8),
      u8(pixel)
    };

    fwrite(rgb, 1, 3, fp);
  }

  fclose(fp);
  return true;
}

static auto normalizeRegion(string region) -> string {
  if(region.find("PAL")) return "PAL";
  return "NTSC";
}

static auto boolString(bool value) -> string {
  return value ? "true" : "false";
}

auto main(int argc, char** argv) -> int {
  Options options;

  if(!parseArgs(argc, argv, options)) {
    usage(argv[0]);
    return 1;
  }

  print("n64-headless config:\n");
  print("  rom: ", options.romPath, "\n");
  print("  frames: ", options.frames, "\n");
  print("  dump: ", options.dumpPath, "\n");
  print("  gpu: ", options.gpu ? "1" : "0", "\n");
  print("  homebrew: ", options.homebrew ? "1" : "0", "\n");
  print("  recompiler: ", options.recompiler ? "1" : "0", "\n");
  print("  expansion pak: ", options.expansionPak ? "1" : "0", "\n");
  print("  controller pak banks: ", options.controllerPakBanks, "\n");

  directory::create("./Saves/");

  mia::setHomeLocation([] { return string{"./"}; });
  mia::setSaveLocation([] { return string{"./Saves/"}; });

  PlatformHeadless headless;
  ares::platform = &headless;

  auto game = mia::Medium::create("Nintendo 64");
  if(!game) {
    print("failed to create Nintendo 64 medium\n");
    return 2;
  }

  auto gameResult = game->load(options.romPath);
  if(gameResult != successful) {
    print("failed to load ROM: ", options.romPath, "\n");
    return 3;
  }

  auto system = mia::System::create("Nintendo 64");
  if(!system) {
    print("failed to create Nintendo 64 system\n");
    return 4;
  }

  auto systemResult = system->load();
  if(systemResult != successful) {
    print("failed to load Nintendo 64 system pak\n");
    return 5;
  }

  headless.gamePak = game->pak;
  headless.systemPak = system->pak;

  string region = "NTSC";
  if(game->pak && game->pak->attribute("region")) {
    region = normalizeRegion(game->pak->attribute("region"));
  }

  ares::Nintendo64::option("Quality", "0");
  ares::Nintendo64::option("Supersampling", "false");
  ares::Nintendo64::option("Enable GPU acceleration", options.gpu);
  ares::Nintendo64::option("Disable Video Interface Processing", false);
  ares::Nintendo64::option("Weave Deinterlacing", false);
  ares::Nintendo64::option("Homebrew Mode", options.homebrew);
  ares::Nintendo64::option("Recompiler", options.recompiler);
  ares::Nintendo64::option("Expansion Pak", options.expansionPak);
  ares::Nintendo64::option("Controller Pak Banks", options.controllerPakBanks);

  ares::Node::System root;

  string systemName = {"[Nintendo] Nintendo 64 (", region, ")"};
  print("loading core system: ", systemName, "\n");

  if(!ares::Nintendo64::load(root, systemName)) {
    print("failed to load core system: ", systemName, "\n");
    return 6;
  }

  if(auto port = root->find<ares::Node::Port>("Cartridge Slot")) {
    port->allocate();
    port->connect();
  } else {
    print("could not find Cartridge Slot\n");
    return 7;
  }

  if(auto port = root->find<ares::Node::Port>("Controller Port 1")) {
    port->allocate("Gamepad");
    port->connect();
  } else {
    print("could not find Controller Port 1\n");
  }

  root->power();

  while(headless.frameCount < options.frames) {
    root->run();
  }

  if(!dumpPPM(options.dumpPath, headless.frame, headless.frameWidth, headless.frameHeight)) {
    print("failed to dump frame\n");
    return 8;
  }

  print("dumped ", headless.frameWidth, "x", headless.frameHeight, " frame to ", options.dumpPath, "\n");

  root->save();
  root->unload();

  game->save(game->location);
  system->save(system->location);

  return 0;
}