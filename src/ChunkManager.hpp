#pragma once

#include <Chunk.hpp>
#include <Shader.hpp>
#include <glm/glm.hpp>

class ChunkManager {
   public:
	ChunkManager(glm::vec3 playerPos);
	~ChunkManager();

	void Update(glm::vec3 playerPos);
	void Draw(Shader s);

   private:
	std::vector<Chunk*> chunks;

	HeightMap heightMap;
};
