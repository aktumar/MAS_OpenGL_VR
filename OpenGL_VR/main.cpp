#define _CRT_SECURE_NO_WARNINGS
#define FILE_NUM 3
 
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

#include <iostream>
#include <iomanip>
#include <fstream> 
#include <vector>

#include <GL/freeglut.h>
#include <GL/glew.h>

#include "Generation.h"
#include "Shader.h"
#include "Mouse.h"

Shader* shaderMain;

GLuint vbo;
GLuint cboObject;
GLuint cboLine;

GLuint posLoc;
GLuint colorObjectLoc;
GLuint colorLineLoc;

std::vector<float> verticesMain;

uintptr_t colorModelSize;
uintptr_t verticesModelSize;
uintptr_t verticesHoleSize;

void myInit()
{
	shaderMain = new Shader("main.vs", "main.frag"); 
	shaderMain->useShader();

	posLoc = glGetAttribLocation(shaderMain->getProgramId(), "pos");
	colorObjectLoc = glGetAttribLocation(shaderMain->getProgramId(), "color");
	colorLineLoc = glGetAttribLocation(shaderMain->getProgramId(), "color");

	viewLoc = glGetUniformLocation(shaderMain->getProgramId(), "view");
	modelLoc = glGetUniformLocation(shaderMain->getProgramId(), "model");
	projectionLoc = glGetUniformLocation(shaderMain->getProgramId(), "projection");

	FileGen(FILE_NUM);

	glm::vec3 ***vertices = new glm::vec3 **[2 * Nz];
	for (int i = 0; i < 2 * Nz; i++) 
	{
		vertices[i] = new glm::vec3 *[2 * Ny];
		for (int j = 0; j < 2 * Ny; j++) 
		{
			vertices[i][j] = new glm::vec3[2 * Nx];
		}
	}

	int wx = 0;
	int wy = 0;
	int wz = 0;

	for (int i = 0; i < Nz; i++)
	{
		for (int i1 = 0; i1 < 2; i1++)//lower and upper planes
		{
			wy = 0;
			for (int j = 0; j < Ny; j++)
			{
				for (int j1 = 0; j1 < 2; j1++)//front and back needles
				{
					wx = 0;
					for (int k = 0; k < Nx; k++)
					{
						for (int k1 = 0; k1 < 2; k1++)//right and left points
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

								float p = (zcorn[wz][wy][wx] - coord[j + j1][k + k1][5]) / check;
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
	
	float division;
	float irr = 0;

	glm::vec3 valueMax;
	glm::vec3 valueMin;
	glm::vec3 valueCentre;

	for (int i = 0; i < 2; i++)
	{
		valueMax = vertices[0][0][0];
		valueMin = vertices[0][0][0];
		for (int i = 0; i < 2 * Nz; i++)
		{
			for (int j = 0; j < 2 * Ny; j++)
			{
				for (int k = 0; k < 2 * Nx; k++)
				{
					if (vertices[i][j][k].x > valueMax.x) 
						valueMax.x = vertices[i][j][k].x;
					if (vertices[i][j][k].x < valueMin.x) 
						valueMin.x = vertices[i][j][k].x;
					if (vertices[i][j][k].y > valueMax.y) 
						valueMax.y = vertices[i][j][k].y;
					if (vertices[i][j][k].y < valueMin.y) 
						valueMin.y = vertices[i][j][k].y;
					if (vertices[i][j][k].z > valueMax.z) 
						valueMax.z = vertices[i][j][k].z;
					if (vertices[i][j][k].z < valueMin.z) 
						valueMin.z = vertices[i][j][k].z;
				}
			}
		}

		if (irr == 0)
		{
			glm::vec3 d = valueMax - valueMin;

			if (d.x >= d.y && d.x >= d.z)
			{
				division = d.x;
			}
			else 
			{ 
				if (d.y >= d.x && d.y >= d.z)
				{
					division = d.y;
				} 
				else 
				{
					division = d.z; 
				}
			}

			for (int i = 0; i < 2 * Nz; i++)
			{
				for (int j = 0; j < 2 * Ny; j++)
				{
					for (int k = 0; k < 2 * Nx; k++)
					{
						vertices[i][j][k] /= division;
					}
				}
			}
			irr++;
		}
		else
		{
			valueCentre = (valueMax + valueMin) / glm::vec3(2, 2, 2);
			for (int i = 0; i < 2 * Nz; i++)
			{
				for (int j = 0; j < 2 * Ny; j++)
				{
					for (int k = 0; k < 2 * Nx; k++)
					{
						vertices[i][j][k].z = (vertices[i][j][k].z - valueMin.z) / (valueMax.z - valueMin.z);
						vertices[i][j][k] -= valueCentre;
					}
				}
			}
		}
	}
	
	int Nxp = 0;
	int Nyp = 0;
	int Nzp = 0;

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
	colorModelSize = colorTriangle.size();
	verticesHoleSize = verticesMain.size() - verticesModelSize;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verticesMain.size() * sizeof(float), verticesMain.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &cboObject);
	glBindBuffer(GL_ARRAY_BUFFER, cboObject);
	glBufferData(GL_ARRAY_BUFFER, colorTriangle.size() * sizeof(float), colorTriangle.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &cboLine);
	glBindBuffer(GL_ARRAY_BUFFER, cboLine);
	glBufferData(GL_ARRAY_BUFFER, colorLine.size() * sizeof(float), colorLine.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//std::ofstream fout("output.txt");

	/*for (int i = 0; i < verticesMain.size(); i=i+3)
	{
		fout << "v " << verticesMain[i];
		fout << " " << verticesMain[i + 1];
		fout << " " << verticesMain[i + 2];
		fout << " " << colorTriangle[i];
		fout << " " << colorTriangle[i+1];
		fout << " " << colorTriangle[i+2]<< std::endl;

		//fout << "vn " << verticesMain[i] + 0.000011;
		//fout << " " << verticesMain[i + 1] + 0.000011;
		//fout << " " << verticesMain[i + 2] + 0.000011 << std::endl;

	}*/
	
	/*std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	std::cout << "VOT = " << std::endl;
	
	for (int i = 0; i < verticesMain.size()/3; i = i + 3)
	{
		fout << "f " << i + 1 << "//" << i + 1<<" ";
		fout << i + 2 << "//" << i + 2 << " ";
		fout << i + 3 << "//" << i + 3 << std::endl;
	}
	*/



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

	for (int i = 0; i < 2 * Nz; i++)
	{
		for (int j = 0; j < 2 * Ny; j++)
		{
			delete[] zcorn[i][j];
		}
		delete[] zcorn[i];
	}
	delete zcorn;

	for (int i = 0; i < Nz; i++)
	{
		for (int j = 0; j < Ny; j++)
		{
			delete[] actnum[i][j];
		}
		delete[] actnum[i];
	}
	delete actnum;

	for (int i = 0; i < Nz; i++)
	{
		for (int j = 0; j < Ny; j++)
		{
			delete[] ntg[i][j];
		}
		delete[] ntg[i];
	}
	delete ntg;

	for (int i = 0; i < 2 * Nz; i++)
	{
		for (int j = 0; j < 2 * Ny; j++)
		{
			delete[] vertices[i][j];
		}
		delete[] vertices[i];
	}
	delete vertices;
}

void myDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	shaderMain->useShader();

	glEnableVertexAttribArray(posLoc);
	glEnableVertexAttribArray(colorObjectLoc);
	glEnableVertexAttribArray(colorLineLoc);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, cboObject);
	glVertexAttribPointer(colorObjectLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, verticesModelSize / 3);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(verticesModelSize * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, cboObject);
	glVertexAttribPointer(colorObjectLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(colorModelSize * sizeof(float)));
	glDrawArrays(GL_TRIANGLES, 0, verticesHoleSize / 3);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, cboLine);
	glVertexAttribPointer(colorLineLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glDrawArrays(GL_LINES, 0, verticesModelSize / 3);
	
	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(colorObjectLoc);
	glDisableVertexAttribArray(colorLineLoc);

	glutSwapBuffers();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(650, 650);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);

	glewInit();
	myInit();

	glutDisplayFunc(myDisplay);
	glutMouseFunc(Mouse::MouseCallback);
	glutMotionFunc(Mouse::CursorPosCallback);

	glutMainLoop();
	delete shaderMain;
}

