#include "platform-headless.hpp"

auto PlatformHeadless::pak(ares::Node::Object node) -> std::shared_ptr<vfs::directory> {
  if(!node) return {};

  if(node->name() == "Nintendo 64") return systemPak;
  if(node->name() == "Nintendo 64 Cartridge") return gamePak;

  return {};
}

auto PlatformHeadless::video(
  ares::Node::Video::Screen,
  const u32* data,
  u32 pitch,
  u32 width,
  u32 height
) -> void {
  if(!data || !width || !height) return;

  frameWidth = width;
  frameHeight = height;
  frame.resize(width * height);

  const u32 stride = pitch >> 2;

  for(u32 y = 0; y < height; y++) {
    memory::copy<u32>(
      frame.data() + y * width,
      data + y * stride,
      width
    );
  }
  
  frameCount++;
}

auto PlatformHeadless::input(ares::Node::Input::Input node) -> void {
  if(!node) return;

  if(auto button = node->cast<ares::Node::Input::Button>()) {
    button->setValue(false);
  }

  if(auto axis = node->cast<ares::Node::Input::Axis>()) {
    axis->setValue(0);
  }

  if(auto trigger = node->cast<ares::Node::Input::Trigger>()) {
    trigger->setValue(0);
  }
}