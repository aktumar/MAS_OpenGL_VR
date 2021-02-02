#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream> 
#include <vector>

float valueMax, valueMin;

float ***coord, ***zcorn, ***actnum, ***ntg;
std::vector<float> colorTri, colorLine;
FILE *ptrfile1, *ptrfile2, *ptrfile3, *ptrfile4;
int Nx, Ny, Nz;

void FileGen(int NUM)
{
	switch (NUM) 
	{
		case 1:
			ptrfile1 = fopen("Data/_3_3_3 Cube/COORD1.txt", "r+");
			ptrfile2 = fopen("Data/_3_3_3 Cube/ZCORN1.txt", "r+");
			ptrfile3 = fopen("Data/_3_3_3 Cube/ACTNUM1.txt", "r+");
			ptrfile4 = fopen("Data/_3_3_3 Cube/NTG1.txt", "r+");
			Nx = 3;
			Ny = 3;
			Nz = 3;
			break;

		case 2:
			ptrfile1 = fopen("Data/_25_77_29 First/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_25_77_29 First/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_25_77_29 First/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_25_77_29 First/NTG.inc", "r+");
			Nx = 25;
			Ny = 77;
			Nz = 29;
			break;

		case 3:
			ptrfile1 = fopen("Data/_33_33_11_Sample/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_33_33_11_Sample/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_33_33_11_Sample/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_33_33_11_Sample/NTG.inc", "r+");
			Nx = 33;
			Ny = 33;
			Nz = 11;
			break;

		case 4:
			ptrfile1 = fopen("Data/_36_77_33_V_Moldabek/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_36_77_33_V_Moldabek/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_36_77_33_V_Moldabek/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_36_77_33_V_Moldabek/PERMX.inc", "r+");
			Nx = 36;
			Ny = 77;
			Nz = 33;
			break;

		case 5:
			ptrfile1 = fopen("Data/_36_77_33_V_Moldabek/withoutBad/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_36_77_33_V_Moldabek/withoutBad/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_36_77_33_V_Moldabek/withoutBad/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_36_77_33_V_Moldabek/withoutBad/PERMX.inc", "r+");
			Nx = 36;
			Ny = 77;
			Nz = 33;
			break;

		case 6:
			ptrfile1 = fopen("Data/_40_120_20_SAIGUP/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_40_120_20_SAIGUP/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_40_120_20_SAIGUP/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_40_120_20_SAIGUP/PERMX.inc", "r+");
			Nx = 40;
			Ny = 120;
			Nz = 20;
			break;

		case 7:
			ptrfile1 = fopen("Data/_40_120_20_SAIGUP/withoutBad/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_40_120_20_SAIGUP/withoutBad/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_40_120_20_SAIGUP/withoutBad/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_40_120_20_SAIGUP/withoutBad/PERMX.inc", "r+");
			Nx = 40;
			Ny = 120;
			Nz = 20;
			break;

		case 8:
			ptrfile1 = fopen("Data/_67_49_10_Bahytzhan/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_67_49_10_Bahytzhan/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_67_49_10_Bahytzhan/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_67_49_10_Bahytzhan/NTG.inc", "r+");
			Nx = 67;
			Ny = 49;
			Nz = 10;
			break;

			//ne rabotaet, delenie na nol` v Uglah
		case 9:
			ptrfile1 = fopen("Data/_90_96_36_ ResInsight/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_90_96_36_ ResInsight/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_90_96_36_ ResInsight/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_90_96_36_ ResInsight/PERMX.inc", "r+");
			Nx = 90;
			Ny = 96;
			Nz = 36;
			break;


			//ne rabotaet, delenie na nol` v Uglah
		case 10:
			ptrfile1 = fopen("Data/_90_96_36_ ResInsight/withoutBad/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_90_96_36_ ResInsight/withoutBad/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_90_96_36_ ResInsight/withoutBad/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_90_96_36_ ResInsight/withoutBad/PERMX.inc", "r+");
			Nx = 90;
			Ny = 96;
			Nz = 36;
			break;

		case 11:
			ptrfile1 = fopen("Data/_100_100_11_Johansen/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_100_100_11_Johansen/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_100_100_11_Johansen/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_100_100_11_Johansen/PERMX.inc", "r+");
			Nx = 100;
			Ny = 100;
			Nz = 11;
			break;

		case 12:
			ptrfile1 = fopen("Data/_100_100_11_Johansen/withoutBad/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_100_100_11_Johansen/withoutBad/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_100_100_11_Johansen/withoutBad/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_100_100_11_Johansen/withoutBad/PERMX.inc", "r+");
			Nx = 100;
			Ny = 100;
			Nz = 11;
			break;

		case 13:
			ptrfile1 = fopen("Data/_100_100_21_Johansen/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_100_100_21_Johansen/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_100_100_21_Johansen/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_100_100_21_Johansen/PERMX.inc", "r+");
			Nx = 100;
			Ny = 100;
			Nz = 21;
			break;

		case 14:
			ptrfile1 = fopen("Data/_100_100_21_Johansen/withoutBad/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_100_100_21_Johansen/withoutBad/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_100_100_21_Johansen/withoutBad/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_100_100_21_Johansen/withoutBad/PERMX.inc", "r+");
			Nx = 100;
			Ny = 100;
			Nz = 21;
			break;

		case 15:
			ptrfile1 = fopen("Data/_149_189_16_Johansen/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_149_189_16_Johansen/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_149_189_16_Johansen/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_149_189_16_Johansen/PERMX.inc", "r+");
			Nx = 149;
			Ny = 189;
			Nz = 16;
			break;

		case 16:
			ptrfile1 = fopen("Data/_149_189_16_Johansen/withoutBad/COORD.inc", "r+");
			ptrfile2 = fopen("Data/_149_189_16_Johansen/withoutBad/ZCORN.inc", "r+");
			ptrfile3 = fopen("Data/_149_189_16_Johansen/withoutBad/ACTNUM.inc", "r+");
			ptrfile4 = fopen("Data/_149_189_16_Johansen/withoutBad/PERMX.inc", "r+");
			Nx = 149;
			Ny = 189;
			Nz = 16;
			break;

		case 17:
			ptrfile1 = fopen("Data/1. 100 100 11/COORD.txt", "r+");
			ptrfile2 = fopen("Data/1. 100 100 11/ZCORN.txt", "r+");
			ptrfile3 = fopen("Data/1. 100 100 11/ACTNUM.txt", "r+");
			ptrfile4 = fopen("Data/1. 100 100 11/PERMX.txt", "r+");
			Nx = 100;
			Ny = 100;
			Nz = 11;
			break;

		default:
			std::cout << "does not exist";
			break;
	}

	/**********************************COORDS.INC********************************************/
	float s1;
	while ((fscanf(ptrfile1, "%f", &s1) != EOF))
	{
		if (!ptrfile1) { break; }
	}
	rewind(ptrfile1);
	coord = new float**[Ny + 1];
	for (int i = 0; i < Ny + 1; i++)
	{
		coord[i] = new float*[Nx + 1];
		for (int j = 0; j < Nx + 1; j++)
		{
			coord[i][j] = new float[6];
			for (int k = 0; k < 6; k++)
			{
				fscanf(ptrfile1, "%f", &coord[i][j][k]);
			}
		}
	}
	/**********************************ZCORN.INC********************************************/
	float s2;
	while ((fscanf(ptrfile2, "%f", &s2) != EOF))
	{
		if (!ptrfile2) { break; }
	}
	rewind(ptrfile2);
	zcorn = new float**[2 * Nz];
	for (int i = 0; i < 2 * Nz; i++)
	{
		zcorn[i] = new float*[2 * Ny];
		for (int j = 0; j < 2 * Ny; j++)
		{
			zcorn[i][j] = new float[2 * Nx];
			for (int k = 0; k < 2 * Nx; k++)
			{
				fscanf(ptrfile2, "%f", &zcorn[i][j][k]);
			}
		}
	}
	/**********************************ACTNUM.INC*******************************************/
	float s3;
	while ((fscanf(ptrfile3, "%f", &s3) != EOF))
	{
		if (!ptrfile3) { break; }
	}
	rewind(ptrfile3);
	actnum = new float**[Nz];
	for (int i = 0; i < Nz; i++)
	{
		actnum[i] = new float*[Ny];
		for (int j = 0; j < Ny; j++)
		{
			actnum[i][j] = new float[Nx];
			for (int k = 0; k < Nx; k++)
			{
				fscanf(ptrfile3, "%f", &actnum[i][j][k]);
			}
		}
	}
	/************************************NTG.INC********************************************/
	float s4;
	while ((fscanf(ptrfile4, "%f", &s4) != EOF))
	{
		if (!ptrfile4) { break; }
	}
	rewind(ptrfile4);
	ntg = new float**[Nz];
	for (int i = 0; i < Nz; i++)
	{
		ntg[i] = new float*[Ny];
		for (int j = 0; j < Ny; j++)
		{
			ntg[i][j] = new float[Nx];
			for (int k = 0; k < Nx; k++)
			{
				fscanf(ptrfile4, "%f", &ntg[i][j][k]);
			}
		}
	}
	valueMax = ntg[0][0][0];
	valueMin = ntg[0][0][0];
	for (int i = 0; i < Nz; i++)
		for (int j = 0; j < Ny; j++)
			for (int k = 0; k < Nx; k++)
			{
				if (ntg[i][j][k] > valueMax)valueMax = ntg[i][j][k];
				if (ntg[i][j][k] < valueMin)valueMin = ntg[i][j][k];
			}
	std::cout << "valueMax = " << valueMax << "    valueMin = " << valueMin << std::endl;
}

void ColorGeneration(float value)
{
	float newValue;
	newValue = (value - valueMin) / (valueMax - valueMin);
	if (newValue <= 0.25)
	{
		colorTri.push_back(0);
		colorTri.push_back(4 * newValue);
		colorTri.push_back(1);
	}
	if (newValue > 0.25 && newValue <= 0.5)
	{
		colorTri.push_back(0);
		colorTri.push_back(1);
		colorTri.push_back((-4)*newValue + 2);
	}
	if (newValue > 0.5 && newValue <= 0.75)
	{
		colorTri.push_back(4 * newValue - 2);
		colorTri.push_back(1);
		colorTri.push_back(0);
	}
	if (newValue > 0.75)
	{
		colorTri.push_back(1);
		colorTri.push_back((-4) * newValue + 4);
		colorTri.push_back(0);
	}
	colorLine.push_back(0);
	colorLine.push_back(0);
	colorLine.push_back(0);
}