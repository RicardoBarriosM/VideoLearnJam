#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Debug.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture.h"
#include "Video.h"
#include "VideoTexture.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 720, "Test Video Player", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Enable V-Sync
    glfwSwapInterval(1);

    GLenum glewInitResult = glewInit();
    if (glewInitResult != GLEW_OK)
    {
        Debug::LogError("GLEW Initialization error: " + std::string((const char*)glewGetErrorString(glewInitResult)));
    }

    Debug::Log("GL VERSION: " + std::string((const char*)glGetString(GL_VERSION)));
    {

        float vertices[] = {
            // x     y      u     v
            -1.0f, -1.0f,  0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  0.0f, 1.0f  // top-left
        };

        unsigned int indices[] =
        {
            0, 1, 2,
            2, 3, 0
        };

        VertexArray vertexArray;
        VertexBuffer vertexBuffer(vertices, 4 * 4 * sizeof(float));

        VertexBufferLayout bufferLayout;
        bufferLayout.Push<float>(2); //positions
        bufferLayout.Push<float>(2); //texture coordinates
        vertexArray.AddBuffer(vertexBuffer, bufferLayout);

        IndexBuffer indexBuffer(indices, 6);

        Shader shader("resources/shaders/unlitTexture.shader");
        shader.Bind();
        shader.SetUniform4f("u_color", 0.2f, 0.3f, 1.0f, 1.0f);

        //Texture texture("resources/textures/valkyrie.png");
        //texture.Bind();
        //shader.SetUniform1i("u_Texture", 0);

        Video video;
        uint8_t* frame_data;

        if (!video.Load("resources/videos/big_buck_bunny.avi"))
        {
            Debug::LogError("Couldn't load video frame");
            return 1;
        }

        video.GetNextFrame(&frame_data);

        VideoTexture videoTexture(video.width, video.height, frame_data);
        videoTexture.Bind();
        shader.SetUniform1i("u_Texture", 0);

        vertexArray.UnBind();
        vertexBuffer.UnBind();
        indexBuffer.UnBind();
        shader.UnBind();

        Renderer renderer;

        double delay = video.GetFrameDelay();
        double lastTime = glfwGetTime();

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            double currentTime = glfwGetTime();
            if (currentTime - lastTime >= delay)
            {
                if (video.GetNextFrame(&frame_data))
                {
                    videoTexture.Update(frame_data);
                }
                lastTime = currentTime;
            }

            /* Render here */
            renderer.Clear();
            videoTexture.Bind();

            shader.Bind();
            shader.SetUniform4f("u_color", 0.2f, 0.3f, 1.0f, 1.0f);

            renderer.Draw(vertexArray, indexBuffer, shader);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }
    glfwTerminate();
    return 0;
}