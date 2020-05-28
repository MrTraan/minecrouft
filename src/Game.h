#pragma once

#include <Camera.hpp>
#include <ChunkManager.hpp>
#include <Hud.hpp>
#include <IO.hpp>
#include <Player.hpp>
#include <Skybox.hpp>
#include <Window.hpp>
#include "packer.h"

constexpr float FIXED_TIMESTEP = 1.0f / 30.0f;

struct Game {
	enum class State {
		MENU,
		PLAYING,
		LOADING,
	};

	State         state;
	Camera        camera;
	Player        player;
	IO            io;
	ChunkManager  chunkManager;
	Skybox        skybox;
	Window        window;
	Hud           hud;
	PackerPackage package;
};

extern Game * theGame;
