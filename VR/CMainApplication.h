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

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <SDL/SDL_opengl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <glm-master/glm/glm.hpp>
#include <glm-master/glm/gtc/matrix_transform.hpp>
#include <glm-master/glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <GL/glut.h>
#include <openvr.h>

#include "Matrices.h"
#include "CGLRenderModel.h"

static bool g_bPrintf = true;
void ThreadSleep(unsigned long nMilliseconds);
void dprintf(const char *fmt, ...);
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);

class CMainApplication
{
public:

	//main1
	bool BInit();

	Matrix4 matDeviceToTracking;
	
	//1.CreateAllShaders();
	bool CreateShaders();
	
	GLuint SceneProgramID;
	GLuint CompanionWindowProgramID;
	GLuint RenderModelProgramID;

	GLint SceneMatrixLocation = -1;
	GLint RenderModelMatrixLocation = -1;

	GLuint CompileShader(const char *pchVertexShader, const char *pchFragmentShader);
	void LoadFile(const char *fn, std::string &str);
	GLuint LoadShader(std::string &source, GLuint mode);

	GLuint vsh;
	GLuint fsh;
	GLuint program;

	//2.SetupScene();
	void SetupScene();

	void AddModelToScene(std::vector<float> &vertdata);
	void FileGeneration(int NUM);
	void ColorGeneration(float value);

	uintptr_t verticesModelSize;
	uintptr_t verticesHoleSize;
	uintptr_t colorModelSize;

	std::vector<float> colorTri;
	std::vector<float> colorLine;

	float valueMin;
	float valueMax;

	float ***coord;
	float ***zcorn;
	float ***actnum;
	float ***ntg;

	FILE *ptrfile1;
	FILE *ptrfile2;
	FILE *ptrfile3;
	FILE *ptrfile4;

	int Nx;
	int Ny;
	int Nz;

	//3.SetupCameras();
	void SetupCameras();
	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	
	//4.SetupStereoRenderTargets();	
	bool SetupStereoRenderTargets();

	//5.SetupCompanionWindow();
	void SetupCompanionWindow();

	GLuint vaoCompanionWindow;
	GLuint vboWind;
	GLuint uvboWind;
	GLuint ivboWind;

	unsigned int indicesWindSize;

	//6.SetupRenderModels();
	void SetupRenderModels();
	void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	CGLRenderModel *FindOrLoadRenderModel(const char *pchRenderModelName);


	//main2
	void RunMainLoop();

	//1.HandleInput();
	bool HandleInput();
	void ProcessVREvent(const vr::VREvent_t & event);
	void ButtonAction(const vr::VREvent_t & event);

	bool isGripPressed;
	bool isTriggerPressed;
	bool isTouchpadPressed;
	bool isTouchpadTouched;
	bool isAppllMenuPressed;
	
	bool firstGripMove;
	bool firstTriggerMove;
	bool firstTouchpadPressMove;
	bool firstTouchpadTouchMove;
	bool firstAppllMenuMove;

	//2.RenderFrame();
	void RenderFrame();

	bool m_bVblank = false;
	bool m_bGlFinishHack = true;

	//2.1.RenderStereoTargets();
	void RenderStereoTargets();
	void RenderScene(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];

	Matrix4 matPrev;

	//2.2.RenderCompanionWindow();
	void RenderCompanionWindow();
	
	//2.3.UpdateHMDMatrixPose();
	void UpdateHMDMatrixPose();
	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

	//main3
	void Shutdown();

private:

	vr::IVRSystem *m_pHMD = NULL;

	// SDL bookkeeping
	SDL_Window * CompanionWindow = NULL;
	uint32_t WindWidth = 1100;
	uint32_t WindHeight = 800;
	SDL_GLContext CompanionContext = NULL;

	// OpenGL bookkeeping

	int TrackedControllerCount = 0;
	int TrackedControllerCount_Last = -1;
	int m_iValidPoseCount = 0;
	int m_iValidPoseCount_Last = -1;

	unsigned int vertNumber = 0;

	GLuint vaoSceneObj;
	GLuint vaoSceneHole;
	GLuint vaoSceneLine;

	GLuint vboMain;
	GLuint cvboobj;
	GLuint cvboline;

	Matrix4 HMDPose;
	Matrix4 projectionLeft;
	Matrix4 projectionRight;

	float nearClip = 0.1f;
	float farClip = 30.0f;

	uint32_t RecomWidth;
	uint32_t RecomHeight;

	struct Framebuffer
	{
		GLuint fbo_hmd_depth;
		GLuint fbo_hmd_texture;
		GLuint fbo_hmd;
		GLuint fbo_display_depth;
		GLuint fbo_display_texture;
		GLuint fbo_display;
	};

	Framebuffer leftEyeDesc;
	Framebuffer rightEyeDesc;

	bool CreateFrameBuffer(int nWidth, int nHeight, Framebuffer &framebuffer);

	bool drawModel = true;

	std::string m_strPoseClasses = "";                            // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class

	std::vector< CGLRenderModel * > m_vecRenderModels;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];
};