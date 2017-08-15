#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_longene_hippo_wificar_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}



#include <jni.h>
#include <string>
#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static int _video_width = -1;
static int _video_height = -1;
//static int _ytid = -1, _utid = -1, _vtid = -1;


static GLuint texYId;
static GLuint texUId;
static GLuint texVId;




static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
                                                    = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

static const char gVertexShader[] =
        "attribute vec4 vPosition;\n"
                "attribute vec2 a_texCoord;\n"
                "varying vec2 tc;\n"
                "void main() {\n"
                "gl_Position = vPosition;\n"
                "tc = a_texCoord;\n"
                "}\n";

/*
"attribute vec4 vPosition;\n"
        "void main() {\n"
        "  gl_Position = vPosition;\n"
        "}\n";
*/

static const char gFragmentShader[] =
        "precision mediump float;\n"
                "uniform sampler2D tex_y;\n"
                "uniform sampler2D tex_u;\n"
                "uniform sampler2D tex_v;\n"
                "varying vec2 tc;\n"
                "void main() {\n"
                "vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
                "vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
                "vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
                "c += V * vec4(1.596, -0.813, 0, 0);\n"
                "c += U * vec4(0, -0.392, 2.017, 0);\n"
                "c.a = 1.0;\n"
                "gl_FragColor = c;\n"
                "}\n";


/*
"precision mediump float;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "}\n";
*/

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                         shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;

}

GLuint gProgram;
GLuint gvPositionHandle;
GLuint gvCoordHandle;
GLuint _yhandle;
GLuint _uhandle;
GLuint _vhandle;



bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    /*
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
         gvPositionHandle);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    return true;
     */



    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    LOGD("_positionHandle = %d " ,gvPositionHandle);
    checkGlError("glGetAttribLocation vPosition");

    gvCoordHandle = glGetAttribLocation(gProgram, "a_texCoord");
    LOGD("_coordHandle = %d " ,gvCoordHandle);
    checkGlError("glGetAttribLocation a_texCoord");


    /*
     * get uniform location for y/u/v, we pass data through these uniforms
     */
    _yhandle = glGetUniformLocation(gProgram, "tex_y");
    LOGD("_yhandle = %d" , _yhandle);
    checkGlError("glGetUniformLocation tex_y");

    _uhandle = glGetUniformLocation(gProgram, "tex_u");
    LOGD("_uhandle = %d" , _uhandle);
    checkGlError("glGetUniformLocation tex_u");

    _vhandle = glGetUniformLocation(gProgram, "tex_v");
    LOGD("_vhandle = %d" , _vhandle);
    checkGlError("glGetUniformLocation tex_v");


    //isProgBuilt = true;
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    return true;

}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
                                      0.5f, -0.5f };

unsigned char * readYUV()
{
    FILE *fp;
    unsigned char * buffer;
    long size = 1280 * 720 * 3 / 2;
    int ret;

    if((fp=fopen("/sdcard/test.yuv","rb"))==NULL)
    {
        LOGD("cant open the file");
        return NULL;
    }

    buffer = new unsigned char[size];
    memset(buffer,'\0',size);
    ret = fread(buffer,size,1,fp);
    LOGD("read %d ", ret);
    fclose(fp);
    return buffer;
}

void renderFrame() {
    LOGD("now start show ..............!!!!!!!!!!!!!!!!!!!!!!!!!");
    /*
    static float grey;
    grey += 0.01f;
    if (grey > 1.0f) {
        grey = 0.0f;
    }
    glClearColor(grey, grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    glDrawArrays(GL_TRIANGLES, 0, 3);
    checkGlError("glDrawArrays");
     */

    int width = 720;
    int height = 1280;
    unsigned char *y = NULL;
    y = readYUV();
    unsigned char *u;
    unsigned char *v;





    u = y + width * height;
    v = u + width * height / 4;
    if(y == NULL)
        return;
    LOGD("read ok ..............!!!!!!!!!!!!!!!!!!!!!!!!! %d %d %d", y[0], u[0], v[0]);
    bool videoSizeChanged = (width != _video_width || height != _video_height);
    if (videoSizeChanged) {
        _video_width = width;
        _video_height = height;
        LOGD("buildTextures videoSizeChanged: w= %d h= %d" , _video_width , _video_height);
    }

    // building texture for Y data
    if (texYId < 0 || videoSizeChanged) {
        /*
        if (_ytid >= 0) {
            LOGD("glDeleteTextures Y");
            glDeleteTextures(1, new int * { _ytid }, 0);
            checkGlError("glDeleteTextures");
        }
         */
        // GLES20.glPixelStorei(GLES20.GL_UNPACK_ALIGNMENT, 1);
        //int * textures = new int[1];
        glGenTextures(1, &texYId);
        checkGlError("glGenTextures");
        //texYId = textures[0];
        LOGD("glGenTextures Y = %d" ,texYId);
    }
    glBindTexture(GL_TEXTURE_2D, texYId);
    checkGlError("glBindTexture");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _video_width, _video_height, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, y);
    checkGlError("glTexImage2D");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // building texture for U data
    if (texUId < 0 || videoSizeChanged) {
        /*
        if (_utid >= 0) {
            LOGD("glDeleteTextures U");
            glDeleteTextures(1, new int[] { _utid }, 0);
            checkGlError("glDeleteTextures");
        }
         */
        //int * textures = new int[1];
        glGenTextures(1, &texUId);
        checkGlError("glGenTextures");
        //_utid = textures[0];
        LOGD("glGenTextures U = %d",texUId);
    }
    glBindTexture(GL_TEXTURE_2D, texUId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _video_width / 2, _video_height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, u);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // building texture for V data
    if (texVId < 0 || videoSizeChanged) {
        /*
        if (_vtid >= 0) {
            LOGD("glDeleteTextures V");
            glDeleteTextures(1, new int[] { _vtid }, 0);
            checkGlError("glDeleteTextures");
        }
         */
        //int * textures = new int[1];
        glGenTextures(1, &texVId);
        checkGlError("glGenTextures");
        //_vtid = textures[0];
        LOGD("glGenTextures V = %d",texVId);
    }
    glBindTexture(GL_TEXTURE_2D, texVId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _video_width / 2, _video_height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



    LOGD("now show  mid ..............!!!!!!!!!!!!!!!!!!!!!!!!!");


    glClearColor(0.3f, 0.4f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);



    //static unsigned char * _vertice_buffer = new unsigned char[1280*720*4];
    //static unsigned char * _coord_buffer = new unsigned char[1280*720*4];

    static float  _vertice_buffer[8] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f}; // fullscreen
    static float  _coord_buffer[8] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, false, 8, _vertice_buffer);
    checkGlError("glVertexAttribPointer mPositionHandle");
    glEnableVertexAttribArray(gvPositionHandle);

    glVertexAttribPointer(gvCoordHandle, 2, GL_FLOAT, false, 8, _coord_buffer);
    checkGlError("glVertexAttribPointer maTextureHandle");
    glEnableVertexAttribArray(gvCoordHandle);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texYId);
    glUniform1i(_yhandle, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texUId);
    glUniform1i(_uhandle, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texVId);
    glUniform1i(_vhandle, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFinish();

    glDisableVertexAttribArray(gvPositionHandle);
    glDisableVertexAttribArray(gvCoordHandle);

    LOGD("now show  end ..............!!!!!!!!!!!!!!!!!!!!!!!!!");
}

//zhuazi test

GLuint gProgram_zhuazi;
GLuint gvPositionHandle_zhuazi;
GLuint gvColor_zhuazi;
GLuint gma_zhuazi;
GLuint gprojection_zhuazi;

static float projection_matrix[16];


static float  model_matrix[16] = {
        0.0f,0.0f
};


static float  add_matrix[16] = {
        0.0f,0.0f
};

void make_projection_matrix(float mat[16],float deg,int w,int h, float n, float f) {
    float as;
    float angleInRadians = (float)(deg * M_PI / 180.0f);
    float a = (float) (1.0 / tan(angleInRadians / 2.0));
    if (w > h)
        as = (float)w / (float)h;
    else
        as = (float)h / (float)w;
    mat[0] = a / as;
    mat[1] = 0.0f;
    mat[2] = 0.0f;
    mat[3] = 0.0f;


    mat[4] = 0.0f;
    mat[5] = a;
    mat[6] = 0.0f;
    mat[7] = 0.0f;

    mat[8] = 0.0f;
    mat[9] = 0.0f;
    mat[10] = -((f+n)/(f-n));
    mat[11] = -1.0f; //位置对么？

    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = -((2.0f * f * n) / (f - n)); //
    mat[15] = 0.0f;

}

void dump_xyzw(float pos[4], float mat[16])
{
    float tmp[4];


}

static const char gZhuazi_VertexShader[] =
        "attribute vec4 vPosition;\n"
                "attribute vec4 a_Color;\n"

                "varying vec4 v_Color;\n"

                "void main() {\n"
                "v_Color = a_Color;\n"
                "gl_Position = vPosition;\n"
                "}\n";

//每个顶点gl_Position  包含 x，y，z，w四个四维位置  默认 0 0 0 w=1
//顶点着色器中attribute 定义变量是可以被取出来和C变量绑定的
//片段着色器中varying定义的变量是可以在顶点着色器和片段着色器间传递的

static const char gZhuazi_FragmentShader[] =
        "precision mediump float;\n"
                "uniform mat4 ma;\n"
                "uniform mat4 projection;\n"
                "varying vec4 v_Color;\n"
                "void main() {\n"
                "gl_FragColor = v_Color * projection;\n"
                "}\n";

//"uniform mat4 ma;\n"
//"uniform vec4 u_Color;\n"
//每个点颜色gl_FragColor 包含 r、g、b、a
//precision mediump float;   代表每个点的精度 可选 lowp mediump  highp  顶点着色器默认为highp   片段一般为mediump  越高越慢，但是效果好
//片段着色器中uniform定义的变量是可以和C绑定的
//片段着色器中varying定义的变量是可以在顶点着色器和片段着色器间传递的

GLuint loadShader_zhuazi(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);//按不同类型创建着色器
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);//把opengl着色器代码载入
        glCompileShader(shader);//编译opengl着色器代码
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);//获得编译结果
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                         shaderType, buf);
                    free(buf);
                    //获取出错信息
                }
                glDeleteShader(shader);//删除创建的着色器
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram_zhuazi(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader_zhuazi(GL_VERTEX_SHADER, pVertexSource);//可以共用，为了清楚单独拿出来
    if (!vertexShader) {
        LOGE("vertexShader err\n");
        return 0;
    }

    GLuint pixelShader = loadShader_zhuazi(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        LOGE("pixelShader err\n");
        return 0;
    }

    GLuint program = glCreateProgram();//创建opengl程序
    if (program) {
        glAttachShader(program, vertexShader);//把顶点着色器贴到程序
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);//把片段着色器贴到程序
        checkGlError("glAttachShader");
        glLinkProgram(program);//链接程序
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);//获得链接状态
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                    //如果出错打印错误
                }
            }
            glDeleteProgram(program);//如果错误删除程序
            program = 0;
        }
    }
    return program;

}

bool setupGraphics_zhuazi(int w, int h) {
    //这个函数主要做了定义顶点，片段着色器，创建一个程序把他们关联起来，最后将C变量和OPenGL变量绑定
    //这个函数做完也不知道要画什么图案，因为还只是一个程序
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram_zhuazi = createProgram_zhuazi(gZhuazi_VertexShader, gZhuazi_FragmentShader);//多个可以共用一个，这里方便注释分开
    if (!gProgram_zhuazi) {
        LOGE("Could not create program.");
        return false;
    }
    //建图，告诉程序希望画什么  VertexShader 定点着色器，用来确定形状 FragmentShader 片段着色器，用来确定渲染，
    //顶点着色器最终需要的是gl_Position 这个内部变量，告诉opengl我们图案有几个顶点，一般都是从左下角0,0开始，逆时针，从左往右从下往上定义顶点
    //片段着色器最终需要gl_FragColor这个内部变量，告诉opengl每个片段都是什么颜色
    //创建好了opengl程序，并且附上了顶点着色器和片段着色器

    gvPositionHandle_zhuazi = glGetAttribLocation(gProgram_zhuazi, "vPosition");
    LOGD("_positionHandle = %d " ,gvPositionHandle_zhuazi);
    checkGlError("glGetAttribLocation vPosition");
    //将opengl中的attribute变量vPosition的索引号码取出来，日后可以根据这个索引号码修改vPosition的值

    gvColor_zhuazi = glGetAttribLocation(gProgram_zhuazi, "a_Color");
    LOGD("gvColor_zhuazi = %d " ,gvColor_zhuazi);
    checkGlError("glGetAttribLocation a_Color");

    gma_zhuazi = glGetUniformLocation(gProgram_zhuazi, "ma");
    LOGD("fa = %d" , gma_zhuazi);
    checkGlError("glGetUniformLocation gma_zhuazi");

    gprojection_zhuazi = glGetUniformLocation(gProgram_zhuazi, "projection");
    LOGD("fa = %d" , gprojection_zhuazi);
    checkGlError("glGetUniformLocation gprojection_zhuazi");

    //将opengl中的uniform变量 的索引号码取出来，可以根据这个索引给u_Color赋值
    //isProgBuilt = true;
    glViewport(0, 0, w, h);//设置View大小
    checkGlError("glViewport");
    return true;

}

void renderFrame_zhuazi() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//设置颜色
    glClear(GL_COLOR_BUFFER_BIT);

    static float  vertice_buffer[256] = {
            -0.25f,-0.5f,-0.25f,   //位置
            0.25f,-0.5f,-0.25f,
            -0.25f,0.5f,-0.25f,
            0.25f,0.5f,-0.25f,

            -0.25f,0.5f,0.25f,
            0.25f,0.5f,0.25f,

            -0.25f,-0.5f,0.25f,
            0.25f,-0.5f,0.25f,

            -0.25f,-0.5f,-0.25f,
            0.25f,-0.5f,-0.25f,


    };
    //顶点真正定义的地方
    static float  _color_buffer[256] = {
            1.0f,0.0f,0.0f,                        //颜色
            1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,

            0.0f,1.0f,0.0f,                        //颜色
            0.0f,1.0f,0.0f,

            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f,

            0.0f,0.5f,0.5f,
            0.0f,0.5f,0.5f,
    };



    static float  r_buffer[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
    static float fixr = 0.0f;


    glUseProgram(gProgram_zhuazi);
    checkGlError("glUseProgram");

    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, false, 0, vertice_buffer);
    checkGlError("glVertexAttribPointer mPositionHandle");
    glEnableVertexAttribArray(gvPositionHandle);


    glVertexAttribPointer(gvColor_zhuazi, 3, GL_FLOAT, false, 0, _color_buffer);
    checkGlError("glVertexAttribPointer gvColor_zhuazi");
    glEnableVertexAttribArray(gvColor_zhuazi);



    //给顶点着色器赋值 vertice_buffer 赋给 opengl中的vPosition
    //第一个参数是要修改的opengl的索引号码
    //第二个参数是一个顶点对应几个分量，我理解最多应该为4 分别是x，y，z，w 其中w对应景深 ，如果传2，则表示2维平面图案，opengl自己会为余下的变量付默认值 0 0 0 1
    //变量类型
    //如果变量为整形有效，浮点忽略
    //这个代表每个点和下个点之间的间距，当vertice_buffer除了位置xyzw还有附加属性时候就要填上附加属性长度  位置xyz 3位 颜色rgb 3位 所以一共6位
    //C程序buff地址
    //glEnableVertexAttribArray  使能顶点
//    fixr = fixr + 0.01f;
//    if(fixr >= 1.0f)
//        fixr = 0.0f;
    //r_buffer[0] = 1.0f;

    make_projection_matrix(projection_matrix,90.0f,640,480,1.0f,10.0f);
    glUniformMatrix4fv(gprojection_zhuazi,1,GL_FALSE,projection_matrix);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //开始画了
    //第一个参数画什么 GL_TRIANGLE_STRIP 画三角（以前面边为参考 123一个  234 一个  345 一个）  GL_TRIANGLE_FAN 画三角（第一个三角形需要三个顶底，后面都拿第一个点作为顶点  123 一个 134 一个 145 一个）GL_TRIANGLE 画三角（三个顶点） GL_LINES 画线
    //第二个参数从第几个顶点开始
    //第三个参数读取几个顶点
    glFinish();


    glDisableVertexAttribArray(gvPositionHandle);
    glDisableVertexAttribArray(gvColor_zhuazi);

}






//extern "C" {
//JNIEXPORT void JNICALL Java_com_example_hippo_wificar_GL2JNIView_init(JNIEnv * env, jobject obj,  jint width, jint height);
//JNIEXPORT void JNICALL Java_com_example_hippo_wificar_GL2JNIView_step(JNIEnv * env, jobject obj);
//};

extern "C" JNIEXPORT void JNICALL Java_com_longene_hippo_wificar_GL2JNIView_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
    setupGraphics(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_com_longene_hippo_wificar_GL2JNIView_step(JNIEnv * env, jobject obj)
{
    renderFrame();
}

extern "C" JNIEXPORT void JNICALL Java_com_longene_hippo_wificar_GL2JNIView_initzhuazi(JNIEnv * env, jobject obj,  jint width, jint height)
{
    setupGraphics_zhuazi(width, height);
}

extern "C" JNIEXPORT void JNICALL Java_com_longene_hippo_wificar_GL2JNIView_stepzhuazi(JNIEnv * env, jobject obj)
{
    renderFrame_zhuazi();
    //glClear(GL_COLOR_BUFFER_BIT);//清除屏幕按照glClearColor颜色填充
}