#pragma once

#include <Shader.hpp>
#include <constants.hpp>
#include <glm/glm.hpp>
#include <vector>

struct VoxelVertex {
	u8    x;
	u8    z;
	u16   y;
	u8 texIndex;
	u8 texX;
	u16 texY;
	// Use 16 bits for y, 8 bits for x and 8 bits for z
	void SetPosition( const glm::i32vec3 & v ) {
		ng_assert( v.x <= 0xff );
		ng_assert( v.z <= 0xff );
		ng_assert( v.y <= 0xffff );
		x = v.x;
		y = v.y;
		z = v.z;
	}
	void SetTexture( int width, int height ) {
		ng_assert( width < 0xff );
		ng_assert( height < 0xffff );
		texX = width;
		texY = height;
	}
};

struct VoxelMesh {
	VoxelVertex * Vertices = nullptr;
	u32      verticesCount = 0;
	u32      verticesAllocated = 0;
	u32      VAO = 0;
	u32      VBO = 0;
};

void PrepareTexturedUnitCube( VoxelMesh * mesh, float textureIndex );
void UpdateTextureIndexOnUnitCube( VoxelMesh * mesh, float textureIndex );

void meshCreateGLBuffers( VoxelMesh * mesh );
void meshDeleteBuffers( VoxelMesh * mesh );
void meshUpdateBuffer( VoxelMesh * mesh );

void meshDraw( VoxelMesh * mesh );
void meshDrawWireframe( VoxelMesh * mesh );
