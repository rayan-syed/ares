#pragma once

#include <ares/ares.hpp>
#include <mia/mia.hpp>

struct PlatformHeadless : ares::Platform {
  std::shared_ptr<vfs::directory> systemPak;
  std::shared_ptr<vfs::directory> gamePak;

  std::vector<u32> frame;
  u32 frameWidth = 0;
  u32 frameHeight = 0;
  u64 frameCount = 0;

  auto pak(ares::Node::Object node) -> std::shared_ptr<vfs::directory> override;
  auto video(ares::Node::Video::Screen screen, const u32* data, u32 pitch, u32 width, u32 height) -> void override;
  auto input(ares::Node::Input::Input node) -> void override;

  auto audio(ares::Node::Audio::Stream) -> void override {}
  auto refreshRateHint(double) -> void override {}
};