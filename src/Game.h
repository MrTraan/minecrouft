#pragma once

#include <Camera.hpp>
#include <ChunkManager.hpp>
#include <IO.hpp>
#include <Player.hpp>
#include <Skybox.hpp>
#include <Window.hpp>

struct Game {
	Camera       camera;
	Player       player;
	IO           io;
	ChunkManager chunkManager;
	Skybox       skybox;
	Window       window;
};

extern Game * theGame;
