#include "CMainApplication.h"

void ThreadSleep(unsigned long nMilliseconds)
{
#if defined(_WIN32)
	::Sleep(nMilliseconds);
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}
void dprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	if (g_bPrintf)
		printf("%s", buffer);

	OutputDebugStringA(buffer);
}
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	//-----------------------------------------------------------------------------
	// Purpose: Helper to get a string from a tracked device property and turn it
	//			into a std::string
	//-----------------------------------------------------------------------------

	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}


//main1
bool CMainApplication::BInit()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	CompanionWindow = SDL_CreateWindow("hellovr", 500, 100, WindWidth, WindHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	CompanionContext = SDL_GL_CreateContext(CompanionWindow);

	glewExperimental = GL_TRUE;
	glewInit();

	std::string strWindowTitle = "MODEL";
	SDL_SetWindowTitle(CompanionWindow, strWindowTitle.c_str());

	CreateShaders();
	SetupScene();
	SetupCameras();
	SetupStereoRenderTargets();
	SetupCompanionWindow();
	SetupRenderModels();

	matDeviceToTracking = Matrix4();

	if (!vr::VRCompositor())
	{
		printf("Compositor initialization failed\n");
		return false;
	}

	return true;
}

//1.CreateAllShaders();
bool CMainApplication::CreateShaders()
{
	//Scene
	SceneProgramID = CompileShader("SHADERS//Scene.vs", "SHADERS//Scene.frag");
	SceneMatrixLocation = glGetUniformLocation(SceneProgramID, "matrix");

	//CompanionWindow
	CompanionWindowProgramID = CompileShader("SHADERS//CompanionWindow.vs", "SHADERS//CompanionWindow.frag");

	//RenderModel
	RenderModelProgramID = CompileShader("SHADERS//render model.vs", "SHADERS//render model.frag");
	RenderModelMatrixLocation = glGetUniformLocation(RenderModelProgramID, "matrix");

	return SceneProgramID != 0
		&& RenderModelProgramID != 0
		&& CompanionWindowProgramID != 0;
}
GLuint CMainApplication::CompileShader(const char *VertexShader, const char *FragmentShader)
{
	std::string source;

	LoadFile(VertexShader, source);
	vsh = LoadShader(source, GL_VERTEX_SHADER);
	source = "";

	LoadFile(FragmentShader, source);
	fsh = LoadShader(source, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vsh);
	glAttachShader(program, fsh);

	glValidateProgram(program);
	glLinkProgram(program);

	return program;
}
void CMainApplication::LoadFile(const char *fn, std::string &str)
{
	std::ifstream in(fn);
	if (!in.is_open())
	{
		std::cout << "The file of shader " << fn << "   cannot be opened\n";
		return;
	}
	char tmp[300];
	while (!in.eof())
	{
		in.getline(tmp, 300);
		str += tmp;
		str += '\n';
	}
}
GLuint CMainApplication::LoadShader(std::string &source, GLuint mode)
{
	GLuint sh = glCreateShader(mode);

	const char* shCode = source.c_str();

	GLint length = strlen(shCode);
	glShaderSource(sh, 1, &shCode, &length);
	glCompileShader(sh);

	GLint success;
	glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar InfoLog[1024];
		glGetShaderInfoLog(sh, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", mode, InfoLog);
	}
	return sh;
}

//2.SetupScene();
void CMainApplication::SetupScene()
{
	std::vector<float> verticesMain;

	AddModelToScene(verticesMain);
	vertNumber = verticesMain.size() / 3;

	//--------------------------------VAO-------Scene-------Object---------------------
	glGenVertexArrays(1, &vaoSceneObj);
	glBindVertexArray(vaoSceneObj);

	glGenBuffers(1, &vboMain);
	glBindBuffer(GL_ARRAY_BUFFER, vboMain);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesMain.size(), verticesMain.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);//vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &cvboobj);
	glBindBuffer(GL_ARRAY_BUFFER, cvboobj);
	glBufferData(GL_ARRAY_BUFFER, colorTri.size() * sizeof(float), colorTri.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);//color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	//--------------------------------VAO-------Scene-------Hole---------------------
	glGenVertexArrays(1, &vaoSceneHole);
	glBindVertexArray(vaoSceneHole);

	glGenBuffers(1, &vboMain);
	glBindBuffer(GL_ARRAY_BUFFER, vboMain);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesMain.size(), verticesMain.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);//vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(verticesModelSize * sizeof(float)));

	glGenBuffers(1, &cvboobj);
	glBindBuffer(GL_ARRAY_BUFFER, cvboobj);
	glBufferData(GL_ARRAY_BUFFER, colorTri.size() * sizeof(float), colorTri.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);//color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(colorModelSize * sizeof(float)));

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	//--------------------------------VAO-------Scene--------Line---------------------
	glGenVertexArrays(1, &vaoSceneLine);
	glBindVertexArray(vaoSceneLine);

	glGenBuffers(1, &vboMain);
	glBindBuffer(GL_ARRAY_BUFFER, vboMain);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesMain.size(), verticesMain.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);//vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &cvboline);
	glBindBuffer(GL_ARRAY_BUFFER, cvboline);
	glBufferData(GL_ARRAY_BUFFER, colorLine.size() * sizeof(float), colorLine.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);//color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void CMainApplication::AddModelToScene(std::vector<float> &vertdataarray)
{
	FileGeneration(fileNum);

	//vertices
	glm::vec3 *** vertices = new glm::vec3 **[2 * Nz];
	for (int i = 0; i < 2 * Nz; i++) 
	{
		vertices[i] = new glm::vec3 *[2 * Ny];
		for (int j = 0; j < 2 * Ny; j++) 
		{
			vertices[i][j] = new glm::vec3[2 * Nx];
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

	float  DIV;
	glm::vec3 max, min, centr;

	for (int irr = 0; irr < 3; irr++)
	{
		max = vertices[0][0][0], min = vertices[0][0][0];
		for (int i = 0; i < 2 * Nz; i++)
		{
			for (int j = 0; j < 2 * Ny; j++)
			{
				for (int k = 0; k < 2 * Nx; k++)
				{
					if (vertices[i][j][k].x > max.x)
					{
						max.x = vertices[i][j][k].x;
					}
					if (vertices[i][j][k].x < min.x) 
					{
						min.x = vertices[i][j][k].x;
					}
					if (vertices[i][j][k].y > max.y) 
					{
						max.y = vertices[i][j][k].y;
					}
					if (vertices[i][j][k].y < min.y) 
					{ 
						min.y = vertices[i][j][k].y; 
					}
					if (vertices[i][j][k].z > max.z) 
					{
						max.z = vertices[i][j][k].z; 
					}
					if (vertices[i][j][k].z < min.z) 
					{
						min.z = vertices[i][j][k].z; 
					}
				}
			}
		}
		if (irr == 0)
		{
			glm::vec3 d = max - min;
			if (d.x >= d.y && d.x >= d.z) 
			{
				DIV = d.x; 
			}
			else 
			{ 
				if (d.y >= d.x && d.y >= d.z) 
				{ 
					DIV = d.y; 
				}
				else 
				{
					DIV = d.z; 
				}
			}

			for (int i = 0; i < 2 * Nz; i++)
			{
				for (int j = 0; j < 2 * Ny; j++)
				{
					for (int k = 0; k < 2 * Nx; k++)
					{
						vertices[i][j][k] /= DIV;
					}
				}
			}
		}
		if (irr == 1)
		{
			centr = (max + min) / glm::vec3(2, 2, 2);
			for (int i = 0; i < 2 * Nz; i++)
			{
				for (int j = 0; j < 2 * Ny; j++)
				{
					for (int k = 0; k < 2 * Nx; k++)
					{
						vertices[i][j][k] -= centr;
					}
				}
			}
		}
		if (irr == 2)
		{
			centr = (max + min) / glm::vec3(2, 2, 2);
			for (int i = 0; i < 2 * Nz; i++)
			{
				for (int j = 0; j < 2 * Ny; j++)
				{
					for (int k = 0; k < 2 * Nx; k++)
					{
						vertices[i][j][k].z = 1 * ((vertices[i][j][k].z - min.z) / (max.z - min.z));
					}
				}
			}
		}
	}

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
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].z);

						for (int num = 0; num < 6; num++)
						{
							ColorGeneration(ntg[i][j][k]);
						}
					}
					if (i == Nz - 1 || actnum[i + 1][j][k] == 0)
					{
						//draw cube lower plane
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);

						for (int num = 0; num < 6; num++)
						{
							ColorGeneration(ntg[i][j][k]);
						}
					}
					if (k == 0 || actnum[i][j][k - 1] == 0)
					{
						//draw cube right plane
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].z);

						for (int num = 0; num < 6; num++)
						{
							ColorGeneration(ntg[i][j][k]);
						}
					}
					if (k == Nx - 1 || actnum[i][j][k + 1] == 0)
					{
						//draw cube left plane
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);

						for (int num = 0; num < 6; num++)
						{
							ColorGeneration(ntg[i][j][k]);
						}
					}
					if (j == 0 || actnum[i][j - 1][k] == 0)
					{
						//draw cube front plane
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp][Nxp].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);

						for (int num = 0; num < 6; num++)
						{
							ColorGeneration(ntg[i][j][k]);
						}
					}
					if (j == Ny - 1 || actnum[i][j + 1][k] == 0)
					{
						//draw cube back plane
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp + 1].z);

						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].x);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].y);
						vertdataarray.push_back(vertices[Nzp][Nyp + 1][Nxp].z);

						for (int num = 0; num < 6; num++)
						{
							ColorGeneration(ntg[i][j][k]);
						}
					}
				}
				Nxp += 2;
			}
			Nyp += 2;
		}
		Nzp += 2;
	}
	verticesModelSize = vertdataarray.size();
	colorModelSize = colorTri.size();

	Nzp = 0;
	for (int i = 0; i < Nz - 1; i++)
	{
		Nyp = 0;
		for (int j = 0; j < Ny; j++)
		{
			Nxp = 0;
			for (int k = 0; k < Nx; k++)
			{
				if (vertices[Nzp + 1][Nyp][Nxp].z != vertices[Nzp + 2][Nyp][Nxp].z ||
					vertices[Nzp + 1][Nyp + 1][Nxp].z != vertices[Nzp + 2][Nyp + 1][Nxp].z ||
					vertices[Nzp + 1][Nyp][Nxp + 1].z != vertices[Nzp + 2][Nyp][Nxp + 1].z ||
					vertices[Nzp + 1][Nyp + 1][Nxp + 1].z != vertices[Nzp + 2][Nyp + 1][Nxp + 1].z)
				{
					//upper hole
					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].x);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].y);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp + 1].z);

					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);

					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].z);

					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].x);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].y);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp].z);

					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].x);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].y);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp][Nxp].z);

					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].x);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].y);
					vertdataarray.push_back(vertices[Nzp + 1][Nyp + 1][Nxp + 1].z);

					//lower hole
					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp + 1].x);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp + 1].y);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp + 1].z);

					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp + 1].x);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp + 1].y);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp + 1].z);

					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp].x);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp].y);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp].z);

					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp].x);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp].y);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp].z);

					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp].x);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp].y);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp][Nxp].z);

					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp + 1].x);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp + 1].y);
					vertdataarray.push_back(vertices[Nzp + 2][Nyp + 1][Nxp + 1].z);

					for (int num = 0; num < 6; num++)
						ColorGeneration(ntg[i][j][k]);
					for (int num = 0; num < 6; num++)
						ColorGeneration(ntg[i + 1][j][k]);
				}
				Nxp += 2;
			}
			Nyp += 2;
		}
		Nzp += 2;
	}
	verticesHoleSize = vertdataarray.size() - verticesModelSize;

	fclose(ptrfile1);
	fclose(ptrfile2);
	fclose(ptrfile3);
	fclose(ptrfile4);
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
void CMainApplication::FileGeneration(int NUM)
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
		ptrfile1 = fopen("Data/_33_33_11_Sample/COORD.inc", "r+");
		ptrfile2 = fopen("Data/_33_33_11_Sample/ZCORN.inc", "r+");
		ptrfile3 = fopen("Data/_33_33_11_Sample/ACTNUM.inc", "r+");
		ptrfile4 = fopen("Data/_33_33_11_Sample/NTG.inc", "r+");
		Nx = 33;
		Ny = 33;
		Nz = 11;
		break;

	default:
		std::cout << "does not exist";
		break;
	}

	/*
	*COORDS.INC
	*/
	float s1;
	while ((fscanf(ptrfile1, "%f", &s1) != EOF))
	{
		if (!ptrfile1) 
		{ 
			break; 
		}
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

	/*
	* ZCORN.INC
	*/

	float s2;
	while ((fscanf(ptrfile2, "%f", &s2) != EOF))
	{
		if (!ptrfile2)
		{
			break;
		}
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

	/*
	* ACTNUM.INC
	*/

	float s3;
	while ((fscanf(ptrfile3, "%f", &s3) != EOF))
	{
		if (!ptrfile3) 
		{ 
			break; 
		}
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

	/*
	* NTG.INC
	*/

	float s4;
	while ((fscanf(ptrfile4, "%f", &s4) != EOF))
	{
		if (!ptrfile4) 
		{
			break;
		}
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
	{
		for (int j = 0; j < Ny; j++)
		{
			for (int k = 0; k < Nx; k++)
			{
				if (ntg[i][j][k] > valueMax)
				{
					valueMax = ntg[i][j][k];
				}
				if (ntg[i][j][k] < valueMin)
				{
					valueMin = ntg[i][j][k];
				}
			}
		}
	}
	std::cout << "valueMax = " << valueMax << "    valueMin = " << valueMin << std::endl;
}
void CMainApplication::ColorGeneration(float value)
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

//3.SetupCameras();
void CMainApplication::SetupCameras()
{
	// все переменные используются в GetCurrentViewProjectionMatrix
	// значения для matMVP; тип переменных Matrix4
	projectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
	projectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
}
Matrix4 CMainApplication::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, nearClip, farClip);

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

//4.SetupStereoRenderTargets();
bool CMainApplication::SetupStereoRenderTargets()
{
	m_pHMD->GetRecommendedRenderTargetSize(&RecomWidth, &RecomHeight);

	CreateFrameBuffer(RecomWidth, RecomHeight, leftEyeDesc);
	CreateFrameBuffer(RecomWidth, RecomHeight, rightEyeDesc);

	return true;
}
bool CMainApplication::CreateFrameBuffer(int Width, int Height, Framebuffer &framebuffer)
{
	//-------------------------------------FRAMEBUFFER FOR HMD--------------------------------------------------------

	//DEPTH
	glGenRenderbuffers(1, &framebuffer.fbo_hmd_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.fbo_hmd_depth);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, Width, Height);

	//TEXTURE
	glGenTextures(1, &framebuffer.fbo_hmd_texture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.fbo_hmd_texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, Width, Height, true);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &framebuffer.fbo_hmd);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo_hmd);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.fbo_hmd_depth);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebuffer.fbo_hmd_texture, 0);

	//-------------------------------------FRAMEBUFFER FOR DISPLAY-------------------------------------------------

	//TEXTURE
	glGenTextures(1, &framebuffer.fbo_display_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer.fbo_display_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &framebuffer.fbo_display);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo_display);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.fbo_display_texture, 0);
	//----------------------------------------------------------------------------------------------------------

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

//5.SetupCompanionWindow();
void CMainApplication::SetupCompanionWindow()
{
	std::vector<float> verticesWind =
	{
		//left eye
		-1, -1, //0
		0, -1, //1
		-1,  1, //2
		0,  1, //3

		//right eye
		0, -1, //4
		1, -1, //5
		0,  1, //6
		1,  1  //7
	};

	std::vector<float> uvWind =
	{
		0, 0, 1, 0, 0, 1, 1, 1,
		0, 0, 1, 0, 0, 1, 1, 1
	};

	unsigned int indicesWind[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6 };
	indicesWindSize = sizeof(indicesWind);

	//--------------------------------VAO-------CompanionWindow------------------------
	glGenVertexArrays(1, &vaoCompanionWindow);
	glBindVertexArray(vaoCompanionWindow);

	glGenBuffers(1, &vboWind);
	glBindBuffer(GL_ARRAY_BUFFER, vboWind);
	glBufferData(GL_ARRAY_BUFFER, verticesWind.size() * sizeof(float), verticesWind.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);//vertices
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &uvboWind);
	glBindBuffer(GL_ARRAY_BUFFER, uvboWind);
	glBufferData(GL_ARRAY_BUFFER, uvWind.size() * sizeof(float), uvWind.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);//texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &ivboWind);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ivboWind);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesWindSize * sizeof(unsigned int), indicesWind, GL_STATIC_DRAW);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//----------------------------------------------------------------------------------
}

//6.SetupRenderModels();
void CMainApplication::SetupRenderModels()
{
	// memset(str,'_',12); // заполнить первые 12 байт символом '_'
	memset(m_rTrackedDeviceToRenderModel, 0, sizeof(m_rTrackedDeviceToRenderModel));

	if (!m_pHMD)
		return;

	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		SetupRenderModelForTrackedDevice(unTrackedDevice);
	}

}
void CMainApplication::SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String);
	CGLRenderModel *pRenderModel = FindOrLoadRenderModel(sRenderModelName.c_str());
	if (!pRenderModel)
	{
		std::string sTrackingSystemName = GetTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		dprintf("Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
	}
	else
	{
		m_rTrackedDeviceToRenderModel[unTrackedDeviceIndex] = pRenderModel;
		m_rbShowTrackedDevice[unTrackedDeviceIndex] = true;
	}
}
CGLRenderModel *CMainApplication::FindOrLoadRenderModel(const char *pchRenderModelName)
{

	//-----------------------------------------------------------------------------
	// Purpose: Finds a render model we've already loaded or loads a new one
	//-----------------------------------------------------------------------------

	CGLRenderModel *pRenderModel = NULL;
	for (std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		if (!strcmp((*i)->GetName().c_str(), pchRenderModelName))
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if (!pRenderModel)
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while (1)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
			if (error != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			dprintf("Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			dprintf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel(pchRenderModelName);
		if (!pRenderModel->BInit(*pModel, *pTexture))
		{
			dprintf("Unable to create GL model from render model %s\n", pchRenderModelName);
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back(pRenderModel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}
	return pRenderModel;
}




//main2
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	while (!bQuit)
	{
		bQuit = HandleInput();
		RenderFrame();
	}
}


//1.HandleInput();
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		if (sdlEvent.type == SDL_QUIT)
		{
			bRet = true;
		}
		else if (sdlEvent.type == SDL_KEYDOWN)
		{
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE
				|| sdlEvent.key.keysym.sym == SDLK_q)
			{
				bRet = true;
			}
			if (sdlEvent.key.keysym.sym == SDLK_c)
			{
				drawModel = !drawModel;
			}
		}
	}

	// Process SteamVR events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}

	return bRet;
}
void CMainApplication::ProcessVREvent(const vr::VREvent_t & event)
{
	//-----------------------------------------------------------------------------
	// Purpose: Processes a single VR event
	//-----------------------------------------------------------------------------

	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
		SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		dprintf("Device %u attached.\n", event.trackedDeviceIndex);
		break;

	case vr::VREvent_TrackedDeviceDeactivated:
		dprintf("Device %u detached.\n", event.trackedDeviceIndex);
		break;

	case vr::VREvent_TrackedDeviceUpdated:
		dprintf("Device %u updated.\n", event.trackedDeviceIndex);
		break;
	}
	ButtonAction(event);
}
void CMainApplication::ButtonAction(const vr::VREvent_t & event)
{
	switch (event.data.controller.button)
	{
	case vr::k_EButton_Grip:
		switch (event.eventType)
		{
		case vr::VREvent_ButtonPress:
			isGripPressed = true;
			firstGripMove = true;
			break;

		case vr::VREvent_ButtonUnpress:
			isGripPressed = false;
			break;
		}
		break;

	case vr::k_EButton_SteamVR_Trigger:
		switch (event.eventType)
		{
		case vr::VREvent_ButtonPress:

			isTriggerPressed = true;
			firstTriggerMove = true;

			break;

		case vr::VREvent_ButtonUnpress:
			isTriggerPressed = false;
			break;
		}
		break;

	case vr::k_EButton_SteamVR_Touchpad:
		switch (event.eventType)
		{
		case vr::VREvent_ButtonPress:
			isTouchpadPressed = true;
			firstTouchpadPressMove = true;
			break;

		case vr::VREvent_ButtonUnpress:
			isTouchpadPressed = false;
			break;

		case vr::VREvent_ButtonTouch:
			isTouchpadTouched = true;
			firstTouchpadTouchMove = true;
			break;

		case vr::VREvent_ButtonUntouch:
			isTouchpadTouched = false;
			break;
		}
		break;

	case vr::k_EButton_ApplicationMenu:
		switch (event.eventType)
		{
		case vr::VREvent_ButtonPress:
			isAppllMenuPressed = true;
			firstAppllMenuMove = true;
			break;

		case vr::VREvent_ButtonUnpress:
			isAppllMenuPressed = false;
			break;
		}
		break;
	}
};


//2.RenderFrame();
void CMainApplication::RenderFrame()
{
	// for now as fast as possible
	if (m_pHMD)
	{
		RenderStereoTargets();
		RenderCompanionWindow();

		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftEyeDesc.fbo_display_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightEyeDesc.fbo_display_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}

	if (m_bVblank && m_bGlFinishHack)
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	SDL_GL_SwapWindow(CompanionWindow);


	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if (TrackedControllerCount != TrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last)
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;
		TrackedControllerCount_Last = TrackedControllerCount;

		dprintf("PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), TrackedControllerCount);
	}

	UpdateHMDMatrixPose();
}

//2.1.RenderStereoTargets();
void CMainApplication::RenderStereoTargets()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// --------------------------Left Eye---------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.fbo_hmd);
	glViewport(0, 0, RecomWidth, RecomHeight);
	RenderScene(vr::Eye_Left);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.fbo_hmd);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.fbo_display);
	glBlitFramebuffer(0, 0, RecomWidth, RecomHeight, 0, 0, RecomWidth, RecomHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// --------------------------Right Eye----------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.fbo_hmd);
	glViewport(0, 0, RecomWidth, RecomHeight);
	RenderScene(vr::Eye_Right);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.fbo_hmd);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.fbo_display);
	glBlitFramebuffer(0, 0, RecomWidth, RecomHeight, 0, 0, RecomWidth, RecomHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
void CMainApplication::RenderScene(vr::Hmd_Eye nEye)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	//SCENE - ПРОРИСОВКА МОДЕЛИ
	if (drawModel)
	{
		glUseProgram(SceneProgramID);

		if (isTouchpadTouched)
		{
			if (firstTouchpadTouchMove)
			{
				matPrev = m_rmat4DevicePose[3];
				firstTouchpadTouchMove = false;
			}
			else
			{
				auto tmp = m_rmat4DevicePose[3] * matPrev.invert();
				matDeviceToTracking = tmp * matDeviceToTracking;
				matPrev = m_rmat4DevicePose[3];
			}
		}

		Matrix4 matMVP = GetCurrentViewProjectionMatrix(nEye) * matDeviceToTracking;
		glUniformMatrix4fv(SceneMatrixLocation, 1, GL_FALSE, matMVP.get());

		glBindVertexArray(vaoSceneObj);
		glDrawArrays(GL_TRIANGLES, 0, vertNumber);
		glBindVertexArray(0);

		glBindVertexArray(vaoSceneLine);
		glDrawArrays(GL_LINES, 0, vertNumber);
		glBindVertexArray(0);
	}

	//RENDER MODEL - ПРОРИСОВКА КОНТРОЛЛЕРОВ
	glUseProgram(RenderModelProgramID);

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rTrackedDeviceToRenderModel[unTrackedDevice] || !m_rbShowTrackedDevice[unTrackedDevice])
			continue;

		const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[unTrackedDevice];
		if (!pose.bPoseIsValid)
			continue;

		const Matrix4 & matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
		Matrix4 matMVP = GetCurrentViewProjectionMatrix(nEye) * matDeviceToTracking;
		glUniformMatrix4fv(RenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());

		m_rTrackedDeviceToRenderModel[unTrackedDevice]->Draw();
	}

	glUseProgram(0);
}
Matrix4 CMainApplication::GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
{
	Matrix4 matMVP;
	if (nEye == vr::Eye_Left)
	{
		matMVP = projectionLeft * HMDPose;
	}
	else if (nEye == vr::Eye_Right)
	{
		matMVP = projectionRight * HMDPose;
	}
	return matMVP;
}

//2.2.RenderCompanionWindow();
void CMainApplication::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, WindWidth, WindHeight);

	glBindVertexArray(vaoCompanionWindow);
	glUseProgram(CompanionWindowProgramID);

	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.fbo_display_texture);
	glDrawElements(GL_TRIANGLES, indicesWindSize / 2, GL_UNSIGNED_INT, nullptr);

	glBindTexture(GL_TEXTURE_2D, rightEyeDesc.fbo_display_texture);
	glDrawElements(GL_TRIANGLES, indicesWindSize / 2, GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
	glUseProgram(0);
}

//2.3.UpdateHMDMatrixPose();
void CMainApplication::UpdateHMDMatrixPose()
{
	if (!m_pHMD)
		return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_rDevClassChar[nDevice] == 0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		HMDPose.invert();
	}
}
Matrix4 CMainApplication::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}


//main3
void CMainApplication::Shutdown()
{
	if (m_pHMD)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	for (std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		delete (*i);
	}
	m_vecRenderModels.clear();

	if (CompanionContext)
	{
		glDeleteBuffers(1, &vboMain);

		if (SceneProgramID)
		{
			glDeleteProgram(SceneProgramID);
		}
		if (RenderModelProgramID)
		{
			glDeleteProgram(RenderModelProgramID);
		}
		if (CompanionWindowProgramID)
		{
			glDeleteProgram(CompanionWindowProgramID);
		}

		glDeleteRenderbuffers(1, &leftEyeDesc.fbo_hmd_depth);
		glDeleteTextures(1, &leftEyeDesc.fbo_hmd_texture);
		glDeleteFramebuffers(1, &leftEyeDesc.fbo_hmd);
		glDeleteTextures(1, &leftEyeDesc.fbo_display_texture);
		glDeleteFramebuffers(1, &leftEyeDesc.fbo_display);

		glDeleteRenderbuffers(1, &rightEyeDesc.fbo_hmd_depth);
		glDeleteTextures(1, &rightEyeDesc.fbo_hmd_texture);
		glDeleteFramebuffers(1, &rightEyeDesc.fbo_hmd);
		glDeleteTextures(1, &rightEyeDesc.fbo_display_texture);
		glDeleteFramebuffers(1, &rightEyeDesc.fbo_display);

		if (vaoCompanionWindow != 0)
		{
			glDeleteVertexArrays(1, &vaoCompanionWindow);
		}
		if (vaoSceneObj != 0)
		{
			glDeleteVertexArrays(1, &vaoSceneObj);
		}
		if (vaoSceneLine != 0)
		{
			glDeleteVertexArrays(1, &vaoSceneLine);
		}
	}

	if (CompanionWindow)
	{
		SDL_DestroyWindow(CompanionWindow);
		CompanionWindow = NULL;
	}

	SDL_Quit();
}
