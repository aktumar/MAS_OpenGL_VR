#define _CRT_SECURE_NO_WARNINGS
#define fileNum 3
 
//1. _3_3_3 Cube
//2. _25_77_29 First
//3. _33_33_11_Sample
//4-5. _36_77_33_V_Moldabek;
//6-7. _40_120_20_SAIGUP
//8. _67_49_10_Bahytzhan
//9-10. _90_96_36_ ResInsight//v Uglah
//11-12. _100_100_11_Johansen
//13-14. _100_100_21_Johansen
//15-16. _149_189_16_Johansen


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include "Shader.h"
#include "Mouse.h"
#include "Generation.h"
#include <iomanip>
#include <fstream> 


shader* mainShader;

GLuint vbo, posLoc;
GLuint cvboobj, colorobjLoc;
GLuint cvboline, colorlineLoc;

std::vector<float> verticesMain;

uintptr_t colorModelSize, verticesModelSize, verticesHoleSize;

void myInit()
{
	mainShader = new shader("main.vs", "main.frag"); mainShader->useShader();

	glClearColor(0.230320f, 0.345f, 0.7865f, 1.0f);

	posLoc = glGetAttribLocation(mainShader->getProgramId(), "pos");
	colorobjLoc = glGetAttribLocation(mainShader->getProgramId(), "color");
	colorlineLoc = glGetAttribLocation(mainShader->getProgramId(), "color");

	viewLoc = glGetUniformLocation(mainShader->getProgramId(), "view");
	modelLoc = glGetUniformLocation(mainShader->getProgramId(), "model");
	projectionLoc = glGetUniformLocation(mainShader->getProgramId(), "projection");

	/***************************************************************************************/
	FileGen(fileNum);
	/***************************************************************************************/

	//VERTICES
	glm::vec3 ***vertices = new glm::vec3 **[2 * Nz];
	for (int i = 0; i < 2 * Nz; i++) {
		vertices[i] = new glm::vec3 *[2 * Ny];
		for (int j = 0; j < 2 * Ny; j++) {
			vertices[i][j] = new glm::vec3[2 * Nx];
			for (int k = 0; k < 2 * Nx; k++) {}
		}
	}

	float p;
	int wx, wy, wz = 0;
	for (int i = 0; i < Nz; i++)
	{
		for (int i1 = 0; i1 < 2; i1++)//for lower and upper planes
		{
			wy = 0;
			for (int j = 0; j < Ny; j++)
			{
				for (int j1 = 0; j1 < 2; j1++)//for front and back needles
				{
					wx = 0;
					for (int k = 0; k < Nx; k++)
					{
						for (int k1 = 0; k1 < 2; k1++)//for right and left pooints
						{
							int check = coord[j + j1][k + k1][2] - coord[j + j1][k + k1][5];
							if (check == 0)
							{
								vertices[wz][wy][wx].x = coord[j + j1][k + k1][0];
								vertices[wz][wy][wx].y = coord[j + j1][k + k1][1];
								vertices[wz][wy][wx].z = zcorn[wz][wy][wx];
							}
							else
							{
								p = (zcorn[wz][wy][wx] - coord[j + j1][k + k1][5]) / check;
								vertices[wz][wy][wx].x = (coord[j + j1][k + k1][3] + (coord[j + j1][k + k1][0] - coord[j + j1][k + k1][3])*p);
								vertices[wz][wy][wx].y = (coord[j + j1][k + k1][4] + (coord[j + j1][k + k1][1] - coord[j + j1][k + k1][4])*p);
								vertices[wz][wy][wx].z = (zcorn[wz][wy][wx]);
							}
							wx++;
						}
					}
					wy++;
				}
			}
			wz++;
		}
	}
	
	//CENTRE AND DIV
	float  DIV, irr = 0;
	glm::vec3 max, min, centr;

	for (int i = 0; i < 2; i++)
	{
		max = vertices[0][0][0], min = vertices[0][0][0];
		for (int i = 0; i < 2 * Nz; i++)
		{
			for (int j = 0; j < 2 * Ny; j++)
			{
				for (int k = 0; k < 2 * Nx; k++)
				{
					if (vertices[i][j][k].x > max.x) max.x = vertices[i][j][k].x;
					if (vertices[i][j][k].x < min.x) min.x = vertices[i][j][k].x;
					if (vertices[i][j][k].y > max.y) max.y = vertices[i][j][k].y;
					if (vertices[i][j][k].y < min.y) min.y = vertices[i][j][k].y;
					if (vertices[i][j][k].z > max.z) max.z = vertices[i][j][k].z;
					if (vertices[i][j][k].z < min.z) min.z = vertices[i][j][k].z;
				}
			}
		}
		if (irr == 0)
		{
			glm::vec3 d = max - min;
			if (d.x >= d.y && d.x >= d.z)DIV = d.x;
			else { if (d.y >= d.x && d.y >= d.z)DIV = d.y; else DIV = d.z; }
			std::cout << "MAXx = " << d.x << std::endl;
			std::cout << "MAXy = " << d.y << std::endl;
			std::cout << "MAXz = " << d.z << std::endl;
			std::cout << "DIV  = " << DIV << std::endl;

			for (int i = 0; i < 2 * Nz; i++)
				for (int j = 0; j < 2 * Ny; j++)
					for (int k = 0; k < 2 * Nx; k++)
						vertices[i][j][k] /= DIV;
			irr++;
		}
		else
		{
			centr = (max + min) / glm::vec3(2, 2, 2);
			for (int i = 0; i < 2 * Nz; i++)
			{
				for (int j = 0; j < 2 * Ny; j++)
				{
					for (int k = 0; k < 2 * Nx; k++)
					{
						vertices[i][j][k].z = (vertices[i][j][k].z - min.z) / (max.z - min.z);//raswirenie po z
						vertices[i][j][k] -= centr;
					}
				}
			}
		}
	}
	
	//verticesMain
	int Nxp, Nyp, Nzp = 0;
	for (int i = 0; i < Nz; i++)
	{
		Nyp = 0;
		for (int j = 0; j < Ny; j++)
		{
			Nxp = 0;
			for (int k = 0; k < Nx; k++)
			{
				if (actnum[i][j][k] == 1)
				{

					if (i == 0 || actnum[i - 1][j][k] == 0)
					{
						//draw cube upper plane

						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

					}
					if (i == Nz - 1 || actnum[i + 1][j][k] == 0)
					{
						//draw cube lower plane
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);


						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);


						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

					}
					if (k == 0 || actnum[i][j][k - 1] == 0)
					{
						//draw cube right plane
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);


					}
					if (k == Nx - 1 || actnum[i][j][k + 1] == 0)
					{
						//draw cube left plane
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

					}
					if (j == 0 || actnum[i][j - 1][k] == 0)
					{
						//draw cube front plane
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

					}
					if (j == Ny - 1 || actnum[i][j + 1][k] == 0)
					{
						//draw cube back plane
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].z);
						ColorGeneration(ntg[i][j][k]);

						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						verticesMain.push_back(vertices[Nzp][Nyp + 1][Nxp].z);
						ColorGeneration(ntg[i][j][k]);

					}
				}
				Nxp += 2;
			}
			Nyp += 2;
		}
		Nzp += 2;
	}


	verticesModelSize = verticesMain.size();
	colorModelSize = colorTri.size();


	verticesHoleSize = verticesMain.size() - verticesModelSize;
	/**************************************************************************************/

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verticesMain.size() * sizeof(float), verticesMain.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	std::ofstream fout("output.txt");

	//std::cout << "VOT = " << std::endl;
	for (int i = 0; i < verticesMain.size(); i=i+3)
	{
		fout << "v " << verticesMain[i];
		fout << " " << verticesMain[i + 1];
		fout << " " << verticesMain[i + 2];
		fout << " " << colorTri[i];
		fout << " " << colorTri[i+1];
		fout << " " << colorTri[i+2]<< std::endl;

		//fout << "vn " << verticesMain[i] + 0.000011;
		//fout << " " << verticesMain[i + 1] + 0.000011;
		//fout << " " << verticesMain[i + 2] + 0.000011 << std::endl;

	}
	
	/*std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	*/

	//int aktum = 0;
	for (int i = 0; i < verticesMain.size()/3; i = i + 3)
	{
		fout << "f " << i + 1 << "//" << i + 1<<" ";
		fout << i + 2 << "//" << i + 2 << " ";
		fout << i + 3 << "//" << i + 3 << std::endl;
		//aktum++;
	}

	glGenBuffers(1, &cvboobj);
	glBindBuffer(GL_ARRAY_BUFFER, cvboobj);
	glBufferData(GL_ARRAY_BUFFER, colorTri.size() * sizeof(float), colorTri.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &cvboline);
	glBindBuffer(GL_ARRAY_BUFFER, cvboline);
	glBufferData(GL_ARRAY_BUFFER, colorLine.size() * sizeof(float), colorLine.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//DELETE
	for (int i = 0; i < Ny + 1; i++)
	{
		for (int j = 0; j < Nx + 1; j++)
		{
			delete[] coord[i][j];
		}
		delete[] coord[i];
	}
	delete coord;
	std::cout << "coord udalilsya uspewno" << std::endl;

	for (int i = 0; i < 2 * Nz; i++)
	{
		for (int j = 0; j < 2 * Ny; j++)
		{
			delete[] zcorn[i][j];
		}
		delete[] zcorn[i];
	}
	delete zcorn;
	std::cout << "zcorn udalilsya uspewno" << std::endl;

	for (int i = 0; i < Nz; i++)
	{
		for (int j = 0; j < Ny; j++)
		{
			delete[] actnum[i][j];
		}
		delete[] actnum[i];
	}
	delete actnum;
	std::cout << "actnum udalilsya uspewno" << std::endl;

	for (int i = 0; i < Nz; i++)
	{
		for (int j = 0; j < Ny; j++)
		{
			delete[] ntg[i][j];
		}
		delete[] ntg[i];
	}
	delete ntg;
	std::cout << "ntg udalilsya uspewno" << std::endl;

	for (int i = 0; i < 2 * Nz; i++)
	{
		for (int j = 0; j < 2 * Ny; j++)
		{
			delete[] vertices[i][j];
		}
		delete[] vertices[i];
	}
	delete vertices;
	std::cout << "vertices udalilsya uspewno" << std::endl;
}

void myDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	mainShader->useShader();

	glEnableVertexAttribArray(posLoc);
	glEnableVertexAttribArray(colorobjLoc);
	glEnableVertexAttribArray(colorlineLoc);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, cvboobj);
	glVertexAttribPointer(colorobjLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, verticesModelSize / 3);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(verticesModelSize * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, cvboobj);
	glVertexAttribPointer(colorobjLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(colorModelSize * sizeof(float)));
	glDrawArrays(GL_TRIANGLES, 0, verticesHoleSize / 3);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, cvboline);
	glVertexAttribPointer(colorlineLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glDrawArrays(GL_LINES, 0, verticesModelSize / 3);
	
	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(colorobjLoc);
	glDisableVertexAttribArray(colorlineLoc);

	glutSwapBuffers();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(650, 650);//512
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);

	glewInit();
	myInit();

	glutDisplayFunc(myDisplay);
	glutMouseFunc(Mouse::MouseCallback);
	glutMotionFunc(Mouse::CursorPosCallback);

	glutMainLoop();
	delete mainShader;
}

