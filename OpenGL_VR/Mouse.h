#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm-master/glm/glm.hpp>
#include <glm-master/glm/gtc/matrix_transform.hpp>
#include <glm-master/glm/gtc/type_ptr.hpp>

static GLuint viewLoc;
static GLuint modelLoc;
static GLuint projectionLoc;

static glm::mat4 view(1);
static glm::mat4 model(1);
static glm::mat4 projection(1);

static glm::vec3 from;
static glm::vec3 to;

static bool isLeftDown;
static bool isRightDown;
static bool isMiddleDown;

static bool firstLeftMove;
static bool firstRightMove;
static bool firstMiddleMove;

static double prevX;
static double prevY;

class Mouse
{
public:
	static glm::vec3 projectToSphere(float x, float y, float radius)
	{
		x /= radius;
		y /= radius;
		float rad2 = x*x + y*y;
		
		if (rad2 > 1.0f)
		{
			float rad = sqrt(rad2);
			x /= rad;
			y /= rad;
			return glm::vec3(x, y, 0.0f);
		}
		else
		{
			float z = sqrt(1 - rad2);
			return glm::vec3(x, y, z);
		}
	}

	static glm::mat4 projectionMatrix(glm::vec3 from_, glm::vec3 to_)
	{
		glm::vec3 from = glm::normalize(from_);
		glm::vec3 to = glm::normalize(to_);

		glm::vec3 v = glm::cross(from, to);
		float  e = glm::dot(from, to);

		if (e > 1.0f - 1.e-9f)
		{
			return glm::mat4(1.0);
		}
		else
		{
			float h = 1.0f / (1.0f + e);
			float mtx[16];
			mtx[0] = e + h * v.x * v.x;
			mtx[1] = h * v.x * v.y + v.z;
			mtx[2] = h * v.x * v.z - v.y;
			mtx[3] = 0.0f;

			mtx[4] = h * v.x * v.y - v.z;
			mtx[5] = e + h * v.y * v.y;
			mtx[6] = h * v.y * v.z + v.x;
			mtx[7] = 0.0f;

			mtx[8] = h * v.x * v.z + v.y;
			mtx[9] = h * v.y * v.z - v.x;
			mtx[10] = e + h * v.z * v.z;
			mtx[11] = 0.0f;

			mtx[12] = 0.0f;
			mtx[13] = 0.0f;
			mtx[14] = 0.0f;
			mtx[15] = 1.0f;

			return glm::mat4(
				mtx[0], mtx[1], mtx[2], mtx[3], 
				mtx[4], mtx[5], mtx[6], mtx[7], 
				mtx[8], mtx[9], mtx[10], mtx[11], 
				mtx[12], mtx[13], mtx[14], mtx[15]
			);
		}
	}

	static void CursorPosCallback(int x, int y)//GLFWwindow* pWindow
	{
		if (isLeftDown)
		{
			if (firstLeftMove)
			{
				prevX = x;
				prevY = y;
				firstLeftMove = false;
				float xpos = 2.0f*static_cast<float>(x) / static_cast<float>(720) - 1.0f;
				float ypos = 1.0f - 2.0f*static_cast<float>(y) / static_cast<float>(720);
				from = projectToSphere(xpos, ypos, 0.8f);
			}
			else
			{
				prevX = x;
				prevY = y;
				float xpos = 2.0f*static_cast<float>(x) / static_cast<float>(720) - 1.0f;
				float ypos = 1.0f - 2.0f*static_cast<float>(y) / static_cast<float>(720);
				to = projectToSphere(xpos, ypos, 0.8f);
				model = projectionMatrix(from, to)*model;
				from = to;
			}
		}
		if (isMiddleDown)
		{
			if (firstMiddleMove)
			{
				prevX = x;
				prevY = y;
				firstMiddleMove = false;
			}
			else
			{
				projection = glm::perspective(45.0f, 1.0f, 0.1f, 100.f);
				view = glm::translate(view, glm::vec3(0, 0, (x - prevX + y - prevY) / 20.0f));
				prevX = x;
				prevY = y;
			}
		}
		if (isRightDown)
		{
			if (firstRightMove)
			{
				prevX = x;
				prevY = y;
				firstRightMove = false;
			}
			else
			{
				view =  glm::translate(view, glm::vec3((x - prevX) / 200.0f, -(y - prevY) / 200.0f, 0));
				prevX = x;
				prevY = y;
			}
		}

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glutPostRedisplay();
	}

	static void MouseCallback(int Button, int Action, int x, int y)
	{
		if (Button == GLUT_LEFT_BUTTON && Action == GLUT_DOWN)
		{
			isLeftDown = true;
			firstLeftMove = true;
		}
		else
		{
			isLeftDown = false;
		}
		if (Button == GLUT_MIDDLE_BUTTON && Action == GLUT_DOWN)
		{
			isMiddleDown = true;
			firstMiddleMove = true;
		}
		else
		{
			isMiddleDown = false;
		}
		if (Button == GLUT_RIGHT_BUTTON && Action == GLUT_DOWN)
		{
			isRightDown = true;
			firstRightMove = true;
		}
		else
		{
			isRightDown = false;
		}
	};
};

