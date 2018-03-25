#include <imgui/imgui.h>
#include <ChunkManager.hpp>

ChunkManager::ChunkManager(glm::vec3 playerPos) {
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	this->chunks.push_back(
	    new Chunk(eBiome::GRASS, chunkPosition, &(this->heightMap)));
}

ChunkManager::~ChunkManager() {
	for (auto c : chunks)
		delete c;
}

void ChunkManager::Update(glm::vec3 playerPos) {
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	ImGui::Text("Chunk Position: %f %f %f\n", chunkPosition.x, chunkPosition.y,
	            chunkPosition.z);

	for (auto c : chunks) {
		if (c->GetPosition() == chunkPosition)
			return;
	}
	printf("Changing chunk\n");

	delete this->chunks.back();
	this->chunks.pop_back();
	this->chunks.push_back(
	    new Chunk(eBiome::GRASS, chunkPosition, &(this->heightMap)));
}

void ChunkManager::Draw(Shader s) {
	for (auto c : chunks)
		c->Draw(s);
}
