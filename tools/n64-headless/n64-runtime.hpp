#pragma once

#include "platform-headless.hpp"

struct N64RuntimeOptions {
  string romPath;
  string dumpPath = "frame.ppm";

  u64 frames = 120;

  bool gpu = true;
  bool homebrew = true;
  bool recompiler = true;
  bool expansionPak = true;

  string controllerPakBanks = "32KiB (Default)";
};

struct N64Runtime {
  auto load(const N64RuntimeOptions& options) -> bool;
  auto power() -> bool;
  auto runUntilFrame(u64 targetFrame) -> void;
  auto save() -> void;
  auto unload() -> void;

  auto framebuffer() const -> const std::vector<u32>&;
  auto width() const -> u32;
  auto height() const -> u32;
  auto frameCount() const -> u64;

private:
  auto normalizeRegion(string region) -> string;

  N64RuntimeOptions options;

  PlatformHeadless platform;

  std::shared_ptr<mia::Pak> game;
  std::shared_ptr<mia::Pak> system;

  ares::Node::System root;

  bool loaded = false;
  bool powered = false;
};