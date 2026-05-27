#include "n64-runtime.hpp"

namespace ares::Nintendo64 {
  auto option(string name, string value) -> bool;
  auto load(Node::System& node, string name) -> bool;
}

auto N64Runtime::normalizeRegion(string region) -> string {
  if(region.find("PAL")) return "PAL";
  return "NTSC";
}

auto N64Runtime::load(const N64RuntimeOptions& runtimeOptions) -> bool {
  options = runtimeOptions;

  directory::create("./Saves/");

  mia::setHomeLocation([] { return string{"./"}; });
  mia::setSaveLocation([] { return string{"./Saves/"}; });

  ares::platform = &platform;

  game = mia::Medium::create("Nintendo 64");
  if(!game) {
    print("failed to create Nintendo 64 medium\n");
    return false;
  }

  auto gameResult = game->load(options.romPath);
  if(gameResult != successful) {
    print("failed to load ROM: ", options.romPath, "\n");
    return false;
  }

  system = mia::System::create("Nintendo 64");
  if(!system) {
    print("failed to create Nintendo 64 system\n");
    return false;
  }

  auto systemResult = system->load();
  if(systemResult != successful) {
    print("failed to load Nintendo 64 system pak\n");
    return false;
  }

  platform.gamePak = game->pak;
  platform.systemPak = system->pak;

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

  string systemName = {"[Nintendo] Nintendo 64 (", region, ")"};
  print("loading core system: ", systemName, "\n");

  if(!ares::Nintendo64::load(root, systemName)) {
    print("failed to load core system: ", systemName, "\n");
    return false;
  }

  if(auto port = root->find<ares::Node::Port>("Cartridge Slot")) {
    port->allocate();
    port->connect();
  } else {
    print("could not find Cartridge Slot\n");
    return false;
  }

  if(auto port = root->find<ares::Node::Port>("Controller Port 1")) {
    port->allocate("Gamepad");
    port->connect();
  } else {
    print("could not find Controller Port 1\n");
  }

  loaded = true;
  return true;
}

auto N64Runtime::power() -> bool {
  if(!loaded) return false;
  if(powered) return true;

  root->power();
  powered = true;
  return true;
}

auto N64Runtime::runUntilFrame(u64 targetFrame) -> void {
  if(!loaded || !powered) return;

  while(platform.frameCount < targetFrame) {
    root->run();
  }
}

auto N64Runtime::save() -> void {
  if(!loaded) return;

  root->save();

  if(game) game->save(game->location);
  if(system) system->save(system->location);
}

auto N64Runtime::unload() -> void {
  if(!loaded) return;

  root->unload();

  loaded = false;
  powered = false;
}

auto N64Runtime::framebuffer() const -> const std::vector<u32>& {
  return platform.frame;
}

auto N64Runtime::width() const -> u32 {
  return platform.frameWidth;
}

auto N64Runtime::height() const -> u32 {
  return platform.frameHeight;
}

auto N64Runtime::frameCount() const -> u64 {
  return platform.frameCount;
}