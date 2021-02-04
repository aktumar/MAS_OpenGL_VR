#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream> 
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

float valueMax;
float valueMin;

float ***arrCoord;
float ***arrZcorn;
float ***arrActnum;
float ***arrNtg;

std::vector<float> colorTriangle;
std::vector<float> colorLine;

FILE *fileCoord;
FILE *fileZcorn;
FILE *fileActnum;
FILE *fileNtg;

int Nx;
int Ny;
int Nz;

void FileGeneration(int n)
{
	switch (n) 
	{
		case 1:
			fileCoord = fopen("Data/_3_3_3 Cube/COORD1.txt", "r+");
			fileZcorn = fopen("Data/_3_3_3 Cube/ZCORN1.txt", "r+");
			fileActnum = fopen("Data/_3_3_3 Cube/ACTNUM1.txt", "r+");
			fileNtg = fopen("Data/_3_3_3 Cube/NTG1.txt", "r+");
			Nx = 3;
			Ny = 3;
			Nz = 3;
			break;

		case 2:
			fileCoord = fopen("Data/_33_33_11_Sample/COORD.inc", "r+");
			fileZcorn = fopen("Data/_33_33_11_Sample/ZCORN.inc", "r+");
			fileActnum = fopen("Data/_33_33_11_Sample/ACTNUM.inc", "r+");
			fileNtg = fopen("Data/_33_33_11_Sample/NTG.inc", "r+");
			Nx = 33;
			Ny = 33;
			Nz = 11;
			break;

		case 3:
			fileCoord = fopen("Data/_149_189_16_Johansen/withoutBad/COORD.inc", "r+");
			fileZcorn = fopen("Data/_149_189_16_Johansen/withoutBad/ZCORN.inc", "r+");
			fileActnum = fopen("Data/_149_189_16_Johansen/withoutBad/ACTNUM.inc", "r+");
			fileNtg = fopen("Data/_149_189_16_Johansen/withoutBad/PERMX.inc", "r+");
			Nx = 149;
			Ny = 189;
			Nz = 16;
			break;

		case 4:
			fileCoord = fopen("Data/1. 100 100 11/COORD.txt", "r+");
			fileZcorn = fopen("Data/1. 100 100 11/ZCORN.txt", "r+");
			fileActnum = fopen("Data/1. 100 100 11/ACTNUM.txt", "r+");
			fileNtg = fopen("Data/1. 100 100 11/PERMX.txt", "r+");
			Nx = 100;
			Ny = 100;
			Nz = 11;
			break;

		default:
			std::cout << "FIle doesn`t exist";
			break;
	}

	/*
	*COORDS.INC
	*/
	
	float s1;
	while ((fscanf(fileCoord, "%f", &s1) != EOF))
	{
		if (!fileCoord) 
		{ 
			break; 
		}
	}
	rewind(fileCoord);

	arrCoord = new float**[Ny + 1];
	for (int i = 0; i < Ny + 1; i++)
	{
		arrCoord[i] = new float*[Nx + 1];
		for (int j = 0; j < Nx + 1; j++)
		{
			arrCoord[i][j] = new float[6];
			for (int k = 0; k < 6; k++)
			{
				fscanf(fileCoord, "%f", &arrCoord[i][j][k]);
			}
		}
	}

	/*
	*ZCORN.INC
	*/
	
	float s2;
	while ((fscanf(fileZcorn, "%f", &s2) != EOF))
	{
		if (!fileZcorn) 
		{ 
			break; 
		}
	}
	rewind(fileZcorn);

	arrZcorn = new float**[2 * Nz];
	for (int i = 0; i < 2 * Nz; i++)
	{
		arrZcorn[i] = new float*[2 * Ny];
		for (int j = 0; j < 2 * Ny; j++)
		{
			arrZcorn[i][j] = new float[2 * Nx];
			for (int k = 0; k < 2 * Nx; k++)
			{
				fscanf(fileZcorn, "%f", &arrZcorn[i][j][k]);
			}
		}
	}

	/*
	*ACTNUM.INC
	*/

	float s3;
	while ((fscanf(fileActnum, "%f", &s3) != EOF))
	{
		if (!fileActnum) 
		{ 
			break; 
		}
	}
	rewind(fileActnum);

	arrActnum = new float**[Nz];
	for (int i = 0; i < Nz; i++)
	{
		arrActnum[i] = new float*[Ny];
		for (int j = 0; j < Ny; j++)
		{
			arrActnum[i][j] = new float[Nx];
			for (int k = 0; k < Nx; k++)
			{
				fscanf(fileActnum, "%f", &arrActnum[i][j][k]);
			}
		}
	}

	/*
	*NTG.INC
	*/

	float s4;
	while ((fscanf(fileNtg, "%f", &s4) != EOF))
	{
		if (!fileNtg) 
		{ 
			break; 
		}
	}
	rewind(fileNtg);

	arrNtg = new float**[Nz];
	for (int i = 0; i < Nz; i++)
	{
		arrNtg[i] = new float*[Ny];
		for (int j = 0; j < Ny; j++)
		{
			arrNtg[i][j] = new float[Nx];
			for (int k = 0; k < Nx; k++)
			{
				fscanf(fileNtg, "%f", &arrNtg[i][j][k]);
			}
		}
	}

	valueMax = arrNtg[0][0][0];
	valueMin = arrNtg[0][0][0];

	for (int i = 0; i < Nz; i++)
	{
		for (int j = 0; j < Ny; j++)
		{
			for (int k = 0; k < Nx; k++)
			{
				if (arrNtg[i][j][k] > valueMax)
				{
					valueMax = arrNtg[i][j][k];
				}
				if (arrNtg[i][j][k] < valueMin)
				{
					valueMin = arrNtg[i][j][k];
				}
			}
		}
	}
}

void ColorGeneration(float valueOld)
{
	float valueNew;

	valueNew = (valueOld - valueMin) / (valueMax - valueMin);
	if (valueNew <= 0.25)
	{
		colorTriangle.push_back(0);
		colorTriangle.push_back(4 * valueNew);
		colorTriangle.push_back(1);
	}
	if (valueNew > 0.25 && valueNew <= 0.5)
	{
		colorTriangle.push_back(0);
		colorTriangle.push_back(1);
		colorTriangle.push_back((-4)*valueNew + 2);
	}
	if (valueNew > 0.5 && valueNew <= 0.75)
	{
		colorTriangle.push_back(4 * valueNew - 2);
		colorTriangle.push_back(1);
		colorTriangle.push_back(0);
	}
	if (valueNew > 0.75)
	{
		colorTriangle.push_back(1);
		colorTriangle.push_back((-4) * valueNew + 4);
		colorTriangle.push_back(0);
	}

	colorLine.push_back(0);
	colorLine.push_back(0);
	colorLine.push_back(0);

}