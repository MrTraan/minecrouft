#pragma once

#include <Camera.hpp>
#include <Player.hpp>
#include <IO.hpp>
#include <ChunkManager.hpp>
#include <Skybox.hpp>

struct Game {
	Camera       camera;
	Player       player;
	IO           io;
	ChunkManager chunkManager;
	Skybox       skybox;
};

extern Game * theGame;
