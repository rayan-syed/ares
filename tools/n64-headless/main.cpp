#include "platform-headless.hpp"
#include <cstdio>
#include <cstdlib>

namespace ares::Nintendo64 {
  auto option(string name, string value) -> bool;
  auto load(Node::System& node, string name) -> bool;
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

auto main(int argc, char** argv) -> int {
  if(argc < 2) {
    print("usage: n64-headless <rom.z64> [frames] [output.ppm]\n");
    return 1;
  }

  string romPath = argv[1];
  u64 framesToRun = argc >= 3 ? strtoull(argv[2], nullptr, 10) : 120;
  string dumpPath = argc >= 4 ? argv[3] : "frame.ppm";

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

  auto gameResult = game->load(romPath);
  if(gameResult != successful) {
    print("failed to load ROM: ", romPath, "\n");
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

  ares::Nintendo64::option("Enable GPU acceleration", true);
  ares::Nintendo64::option("Homebrew Mode", true);
  ares::Nintendo64::option("Recompiler", true);
  ares::Nintendo64::option("Expansion Pak", true);
  ares::Nintendo64::option("Controller Pak Banks", "32KiB (Default)");

  ares::Node::System root;

  string systemName = {"[Nintendo] Nintendo 64 (", region, ")"};
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

  while(headless.frameCount < framesToRun) {
    root->run();
  }

  if(!dumpPPM(dumpPath, headless.frame, headless.frameWidth, headless.frameHeight)) {
    print("failed to dump frame\n");
    return 8;
  }

  print("dumped ", headless.frameWidth, "x", headless.frameHeight, " frame to ", dumpPath, "\n");

  root->save();
  root->unload();

  game->save(game->location);
  system->save(system->location);

  return 0;
}