/**
 * Author: Erik Smistad
 * www.thebigblob.com
**/

#include <cmath>
#include "marchingcubes.h"
vector<vertex> vertexList;

uchar *** parseRawFile(char * filename, int sizeX, int sizeY, int sizeZ) {
    uchar *** voxels = new uchar**[sizeX];
    FILE * file = fopen(filename, "rb");
    if(file == NULL) {
        printf("File not found: %s", filename);
        exit(-1);
    }

    for(int x = 0; x < sizeX; x++) {
        voxels[x] = new uchar*[sizeY];
        for(int y = 0; y < sizeY; y++) {
            voxels[x][y] = new uchar[sizeZ];
            fread(voxels[x][y], sizeof(uchar), sizeZ, file);
        }
    }
    return voxels;
}

vertex interpolate(double isolevel, vertex p1, vertex p2, int valp1, int valp2) {
    if(fabs(isolevel - valp1) < 0.00001)
        return p1;
    if(fabs(isolevel - valp2) < 0.00001)
        return p2;
    if(fabs(valp1 - valp2) < 0.00001)
        return p1;

    vertex p;
    double diff = (double)(isolevel - valp1) / (valp2 - valp1);
    p.x = p1.x + diff * (p2.x - p1.x);
    p.y = p1.y + diff * (p2.y - p1.y);
    p.z = p1.z + diff * (p2.z - p1.z);

    p.normal_x = p1.normal_x + diff * (p2.normal_x - p1.normal_x);
    p.normal_y = p1.normal_y + diff * (p2.normal_y - p1.normal_y);
    p.normal_z = p1.normal_z + diff * (p2.normal_z - p1.normal_z);

    return p;
}

void processCube(cube cube, double isolevel) {
    int cubeindex = 0;
    if(cube.val[0] > isolevel) cubeindex |= 1;
    if(cube.val[1] > isolevel) cubeindex |= 2;
    if(cube.val[2] > isolevel) cubeindex |= 4;
    if(cube.val[3] > isolevel) cubeindex |= 8;
    if(cube.val[4] > isolevel) cubeindex |= 16;
    if(cube.val[5] > isolevel) cubeindex |= 32;
    if(cube.val[6] > isolevel) cubeindex |= 64;
    if(cube.val[7] > isolevel) cubeindex |= 128;

    // Cube is entirely in/out of the surface
    if(edgeTable[cubeindex] == 0 || edgeTable[cubeindex] == 255)
        return;

    vertex vertlist[12];
    // Find the vertices where the surface intersects the cube
    if(edgeTable[cubeindex] & 1)
        vertlist[0] = interpolate(isolevel,cube.p[0],cube.p[1],cube.val[0],cube.val[1]);
    if(edgeTable[cubeindex] & 2)
        vertlist[1] = interpolate(isolevel,cube.p[1],cube.p[2],cube.val[1],cube.val[2]);
    if(edgeTable[cubeindex] & 4)
        vertlist[2] = interpolate(isolevel,cube.p[2],cube.p[3],cube.val[2],cube.val[3]);
    if(edgeTable[cubeindex] & 8)
        vertlist[3] = interpolate(isolevel,cube.p[3],cube.p[0],cube.val[3],cube.val[0]);
    if(edgeTable[cubeindex] & 16)
        vertlist[4] = interpolate(isolevel,cube.p[4],cube.p[5],cube.val[4],cube.val[5]);
    if(edgeTable[cubeindex] & 32)
        vertlist[5] = interpolate(isolevel,cube.p[5],cube.p[6],cube.val[5],cube.val[6]);
    if(edgeTable[cubeindex] & 64)
        vertlist[6] = interpolate(isolevel,cube.p[6],cube.p[7],cube.val[6],cube.val[7]);
    if(edgeTable[cubeindex] & 128)
        vertlist[7] = interpolate(isolevel,cube.p[7],cube.p[4],cube.val[7],cube.val[4]);
    if(edgeTable[cubeindex] & 256)
        vertlist[8] = interpolate(isolevel,cube.p[0],cube.p[4],cube.val[0],cube.val[4]);
    if(edgeTable[cubeindex] & 512)
        vertlist[9] = interpolate(isolevel,cube.p[1],cube.p[5],cube.val[1],cube.val[5]);
    if(edgeTable[cubeindex] & 1024)
        vertlist[10] = interpolate(isolevel,cube.p[2],cube.p[6],cube.val[2],cube.val[6]);
    if(edgeTable[cubeindex] & 2048)
        vertlist[11] = interpolate(isolevel,cube.p[3],cube.p[7],cube.val[3],cube.val[7]);

    for(int i = 0; triTable[cubeindex][i] != -1; i++) {
        vertexList.push_back(vertlist[triTable[cubeindex][i]]);
    }
}

vector<vertex> runMarchingCubes(double ***voxels, int sizeX, int sizeY, int sizeZ, 
        int stepX, int stepY, int stepZ, double isovalue) {
    // Run the processCube function on every cube in the grid
	for(int x = stepX; x < sizeX-2*stepX; x += stepX) {
		for(int y = stepY; y < sizeY-2*stepY; y += stepY) {
			for(int z = stepZ; z < sizeZ-2*stepZ; z += stepZ) {
				cube c = {{
					{x,y,z, 
                        (double)(voxels[x+stepX][y][z]-voxels[x-stepX][y][z]) / -stepX,
                        (double)(voxels[x][y+stepY][z]-voxels[x][y-stepY][z]) / -stepY,
                        (double)(voxels[x][y][z+stepZ]-voxels[x][y][z-stepZ]) / -stepZ
                    },
					{x+stepX,y,z, 
                        (double)(voxels[x+2*stepX][y][z]-voxels[x][y][z]) / -stepX,
                        (double)(voxels[x+stepX][y+stepY][z]-voxels[x+stepX][y-stepY][z]) / -stepY,
                        (double)(voxels[x+stepX][y][z+stepZ]-voxels[x+stepX][y][z-stepZ]) / -stepZ
                    },
					{x+stepX,y,z+stepZ, 
                        (double)(voxels[x+2*stepX][y][z+stepZ]-voxels[x][y][z+stepZ]) / -stepX,
                        (double)(voxels[x+stepX][y+stepY][z+stepZ]-voxels[x+stepX][y-stepY][z+stepZ]) / -stepY,
                        (double)(voxels[x+stepX][y][z+2*stepZ]-voxels[x+stepX][y][z]) / -stepZ
                    },
					{x,y,z+stepZ, 
                        (double)(voxels[x+stepX][y][z+stepZ]-voxels[x-stepX][y][z+stepZ]) / -stepX,
                        (double)(voxels[x][y+stepY][z+stepZ]-voxels[x][y-stepY][z+stepZ]) / -stepY,
                        (double)(voxels[x][y][z+2*stepZ]-voxels[x][y][z]) / -stepZ
                    },
					{x,y+stepY,z, 
                        (double)(voxels[x+stepX][y+stepY][z]-voxels[x-stepX][y+stepY][z]) / -stepX,
                        (double)(voxels[x][y+2*stepY][z]-voxels[x][y][z]) / -stepY,
                        (double)(voxels[x][y+stepY][z+stepZ]-voxels[x][y+stepY][z-stepZ]) / -stepZ
                    },
					{x+stepX,y+stepY,z, 
                        (double)(voxels[x+2*stepX][y+stepY][z]-voxels[x+stepX][y+stepY][z]) / -stepX,
                        (double)(voxels[x+stepX][y+2*stepY][z]-voxels[x+stepX][y][z]) / -stepY,
                        (double)(voxels[x+stepX][y+stepY][z+stepZ]-voxels[x+stepX][y+stepY][z-stepZ]) / -stepZ
                    },
					{x+stepX,y+stepY,z+stepZ, 
                        (double)(voxels[x+2*stepX][y+stepY][z+stepZ]-voxels[x][y+stepY][z+stepZ]) / -stepX,
                        (double)(voxels[x+stepX][y+2*stepY][z+stepZ]-voxels[x+stepX][y][z+stepZ]) / -stepY,
                        (double)(voxels[x+stepX][y+stepY][z+2*stepZ]-voxels[x+stepX][y+stepY][z]) / -stepZ
                    },
					{x,y+stepY,z+stepZ, 
                        (double)(voxels[x+stepX][y+stepY][z+stepZ]-voxels[x-stepX][y+stepY][z+stepZ]) / -stepX,
                        (double)(voxels[x][y+2*stepY][z+stepZ]-voxels[x][y][z+stepZ]) / -stepY,
                        (double)(voxels[x][y+stepY][z+2*stepZ]-voxels[x][y+stepY][z]) / -stepZ
                    }
				},{
					voxels[x][y][z],
					voxels[x+stepX][y][z],
					voxels[x+stepX][y][z+stepZ],
					voxels[x][y][z+stepZ],
					voxels[x][y+stepY][z],
					voxels[x+stepX][y+stepY][z],
					voxels[x+stepX][y+stepY][z+stepZ],
					voxels[x][y+stepY][z+stepZ]
				}};
				processCube(c, isovalue);
			}
		}
	}

	return vertexList;
}
