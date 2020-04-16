#pragma once

#include <Camera.hpp>
#include <ChunkManager.hpp>
#include <Hud.hpp>
#include <IO.hpp>
#include <Player.hpp>
#include <Skybox.hpp>
#include <Window.hpp>

struct Game {
	enum class State {
		MENU,
		PLAYING,
		LOADING,
	};

	State        state;
	Camera       camera;
	Player       player;
	IO           io;
	ChunkManager chunkManager;
	Skybox       skybox;
	Window       window;
	Hud          hud;
};

extern Game * theGame;
