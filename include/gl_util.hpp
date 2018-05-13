/*
 * gl_util.cpp
 *
 *  Created on: Nov 24, 2011
 *      Author: Stou Sandalski (stou@icapsid.net)
 *     License: Apache 2.0
 *     Version: 7.0 (30 Dec 2016)
 *
 * Description:
 *
 *  Miscellaneous code for graphics. No external dependencies except C++ 2011 and OpenGL
 *
 *  - Quaternions
 *  - Simple 4x4 Matrix math
 *  - OpenGL Camera management (perspective, orthographic, unproject)
 *  - OpenGL shader building (from files or source)
 *  - Timer function
 *  - anti-aliasing aaData_
 *
 *
 * Note about memory layout ( https://www.opengl.org/archives/resources/faq/technical/transformations.htm )
 *
 * In OpenGL a matrix like this:
 *
 *  a.x a.x c.x d.x
 *  a.y b.y c.y d.y
 *  a.z b.z c.z d.z
 *    0   0   0   1
 *
 *  is laid out in memory like this:
 * *
 *  { a.x a.y a.z 0; b.x b.y b.z 0; c.x c.y c.z 0 ; d.x d.y d.z d.w }
 *
 *  { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, transX, transY, transZ, 1 }.
 *
 * http://stackoverflow.com/questions/17717600/confusion-between-c-and-opengl-matrix-order-row-major-vs-column-major
 *
 *
 */


#ifndef HPP_NEWBERRY_GL_UTIL
#define HPP_NEWBERRY_GL_UTIL

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#define USE_EXCEPTIONS 0
#define USE_GLEW 1
#define USE_QT 0

#define GL_GLEXT_PROTOTYPES

#if USE_GLEW
#define GLEW_STATIC
#include "GL/glew.h"
#elif USE_QT
#include <QOpenGLFunctions_3_3_Core>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif


#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

namespace timing {

#ifdef _MSC_VER
    struct timeval {
        long	tv_sec;		/* seconds */
        long	tv_usec;	/* and microseconds */
    };
#endif

  template<class T>
  struct Timer {

    timeval a;
    timeval b;
    timeval result;

    Timer() {
      a.tv_sec = a.tv_usec = 0;
      b.tv_sec = b.tv_usec = 0;
      result.tv_sec = result.tv_usec = 0;

      start();
    }

    void start() {
#ifndef _MSC_VER
        gettimeofday(&a, NULL);
#endif
    }

    T lap() {
#ifndef _MSC_VER
        gettimeofday(&b, NULL);
        timersub(&b, &a, &result);
#endif

      return get();
    }

    /**
     *
     * Return the time in seconds
     *
     */
    T get() {
      return T(result.tv_sec + result.tv_usec / 1000000.0);
    }

    T getStart() {
      return T(a.tv_sec + a.tv_usec / 1000000.0);
    }

    T getEnd() {
      return T(b.tv_sec + b.tv_usec / 1000000.0);
    }

    int getStartInt() {
      return a.tv_sec;
    }

    int getEndInt() {
      return b.tv_sec;
    }
  };
};


/**
 *
 * Return the contents of the file `filename` as a std::string. Useful for
 * loading shaders 
 *
 */
inline std::string load_source(const char *filename){
  using namespace std;

  std::cout<<"Loading "<<filename<<endl;

  ifstream file(filename);

#ifdef USE_EXCEPTIONS
  file.exceptions(ifstream::failbit | ifstream::badbit);
#endif

  string prog(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));

  return prog;
}

/**
 *
 * @param tag
 * @return  True if everything is ok
 */
inline bool check_GL_error(const char *tag){

  GLenum error_code = glGetError();

  if(error_code != GL_NO_ERROR){
    std::cerr << "# OpenGL Error - " << tag << "\t - "
              << gluErrorString(error_code)
              << std::endl;

#ifdef USE_EXCEPTIONS
    throw error_code;
#endif
  }

  return (error_code == GL_NO_ERROR);
}

/**
 *
 * @param shader_type
 * @param file_name
 * @param shader
 * @return  true if everything worked out
 */
inline bool compile_shader(GLenum shader_type, const char* name, const char* source, GLuint& shader){

  using namespace std;

  cout << "Compiling " << name <<" : ";

  shader = glCreateShader(shader_type);

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint result;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

  if(GL_FALSE == result){
    cerr << "FAIL: " << name << endl;
  }else{
    cout << "ok" << endl;
  }

  GLint logLen;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

  if(logLen > 10){
    char *log = new char[logLen];
    GLsizei written;
    glGetShaderInfoLog(shader, logLen, &written, log);
    cerr << "Shader log:" << endl
         << log << endl;

#ifdef USE_EXCEPTIONS
    if(GL_FALSE == result){
      throw result;
    }
#endif

    delete[] log;
  }

  return GL_TRUE == result;
}

/**
 *
 * Build a GLSL program given the sources
 *
 *
 * @param program
 * @param vert_filename
 * @param frag_filename
 * @param geo_filename
 * @return
 */
inline bool build_program_from_source(GLuint program,
                                      const char* name,
                                      const char* vert_source,
                                      const char* frag_source,
                                      const char* geo_source = 0){

  using namespace std;

  GLuint vert_shader;
  GLuint frag_shader;
  GLuint geo_shader;

  if(!compile_shader(GL_VERTEX_SHADER, name, vert_source, vert_shader)){
    return false;
  };

  glAttachShader(program, vert_shader);

  if(!compile_shader(GL_FRAGMENT_SHADER, name, frag_source, frag_shader)){
    return false;
  };

  glAttachShader(program, frag_shader);

  if(geo_source != 0){
    if(!compile_shader(GL_GEOMETRY_SHADER_EXT, name, geo_source, geo_shader)){
      return false;
    }
    glAttachShader(program, geo_shader);
  }

  cout<<"Linking "<<endl;
  glLinkProgram(program);

  GLint logLen;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

  if(logLen > 10){
    char *log = new char[logLen];
    GLsizei written;
    glGetProgramInfoLog(program, logLen, &written, log);
    cerr << "Linker log:" << endl
         << log << endl;
    delete[] log;

#ifdef USE_EXCEPTIONS
//    if(GL_FALSE == result){
//      throw result;
//    }
#endif

  }

  check_GL_error("build_program_from_source: Linking");

  //Don't leak shaders either.
  glDetachShader(program, vert_shader);
  glDetachShader(program, frag_shader);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  if(geo_source != 0){
    glDeleteShader(geo_shader);
  }

  return true;
}

/**
 *
 * Build a GLSL program given the sources
 *
 *
 * @param program
 * @param vert_filename
 * @param frag_filename
 * @param geo_filename
 * @return
 */
inline bool build_program(GLuint program,
                          const char* program_name,
                          const char* vert_filename,
                          const char* frag_filename,
                          const char* geo_filename = 0){

  using namespace std;

  const char* geo_source = 0;

  string vert_src_str = load_source(vert_filename);
  string frag_src_str = load_source(frag_filename);
  string geo_src_str;

  if(geo_filename){
    geo_src_str = load_source(frag_filename);
    geo_source = geo_src_str.c_str();
  }

  return build_program_from_source(program, program_name, vert_src_str.c_str(), frag_src_str.c_str(), geo_source);
}

inline void print_OpenGL_info(){
  using namespace std;
  cout << "OpenGL GL_VERSION " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL GL_VENDOR " << glGetString(GL_VENDOR) << endl;
  cout << "OpenGL GL_RENDERER " << glGetString(GL_RENDERER) << endl;
  cout << "OpenGL GL_SHADING_LANGUAGE_VERSION " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

/**
 * Math functions
 */

/**
 * Matrix Routines
 */


/**
 *
 * Invert a 4x4 matrix
 *
 * @param m  input matrix
 * @param out output matrix
 * @return false if matrix is singular
 */

template <class T>
inline void print_4x4(const T m[16]){

  for(int i = 0; i < 4; ++i){
    for(int j = 0; j < 4; ++j){
      std::cout<<m[4 * j + i]<<", ";
    }
    std::cout<<std::endl;
  }
}

inline void print_1xN(const int N, const float v[]){

  for(int i = 0; i < N; ++i){
    std::cout<<v[i]<<", ";
  }
  std::cout<<std::endl;
}


inline void print_1x3(const float v[]){
  print_1xN(3, v);
}


/**
 * Print a xy-plane slice at the z-index of `iz`
 *
 * @param nx
 * @param ny
 * @param iz
 * @param b
 */
template<class T>
inline void print_z_slice(int nx, int ny, int iz, const T b[]) {

  using namespace std;

  // Go to the beginning of that slice
  int offset = nx * ny * iz;

  for(int j = 0; j < ny; ++j) {
    for(int i = 0; i < nx; ++i) {
      cout << setw(10) << b[offset++] << ", ";
    }
    cout << endl;
  }
}

template<class T>
inline bool invert_4x4(const T m[], T out[]){
// Routine comes from
// http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
// which was taken from Mesa3D

  T inv[16];

  inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15]
           + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

  inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15]
           - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

  inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15]
           + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

  inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
           m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

  inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15]
           - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

  inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15]
           + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

  inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
           m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

  inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15]
           + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

  inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
           m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

  inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15]
           - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

  inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
            m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

  inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
            m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

  inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14]
            - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

  inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14]
            + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

  inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
            m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

  inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

  T det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if(det == 0){
    return false;
  }

  T over_det = T(1) / det;

  for(int i = 0; i < 16; ++i){
    out[i] = inv[i] * over_det;
  }

  return true;
}



/**
 *
 * Matrix multiply A and B
 *
 * @tparam T
 * @param a
 * @param b
 * @param out
 * @return
 */
template<class T>
inline bool mul_4x4(const T a[], const T b[], T out[]){

/*
  int offset_out = 0;

  // Go through the rows
  for(int j = 0; j < 4; ++j){

    v_out[j] = 0.0;
    // Go through the columns
    for(int i = 0; i < 4; ++i){
      v_out[j] += m[4 * i + j] * v_in[i];

//      cout<<" j, i, m_offset "<<j<<", "<<i<<", "<<4 * i + j<<endl;
    }
  }
*/



  return true;
}



/**
 *
 *
 *
 * @param m  A 16 element array representing a matrix multiplication
 * @param v_in  A 4 element array
 * @param v_out  A 4 element array
 */

template <class T>
inline void vec4_by_mat4x4(const T m[], const T* v_in, T* v_out){

  using namespace std;

  // Go through the rows
  for(int j = 0; j < 4; ++j){

    v_out[j] = 0.0;
    // Go through the columns
    for(int i = 0; i < 4; ++i){
      v_out[j] += m[4 * i + j] * v_in[i];

//      cout<<" j, i, m_offset "<<j<<", "<<i<<", "<<4 * i + j<<endl;
    }
  }

#if 0
  std::cout<<"m "<<std::endl;
  print_4x4(m);
  std::cout<<"v_in "<<std::endl;
  print_1xN(4, v_in);
  std::cout<<"v_out "<<std::endl;
  print_1xN(4, v_out);
#endif
}

/**
 *
 * Quaternion Routines
 *
 */

/**
 *
 * Create a quaternion that will rotate vector u to the position of vector v
 *
 * http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors
 *
 */
template <class T>
inline void q_from_two_v(T u[], T v[], T q_out[]){

  using namespace std;

//  cout<<"v length "<<sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])<<endl;
//  cout<<"u length "<<sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2])<<endl;


  float dot = (u[0] * v[0] + u[1] * v[1] + u[2] * v[2]);

//  cout<<" Angle between u, v "<<acos(dot)<<endl;

  // This Woodwardism is written under the assumption that the hardware does sqrt by
  // 1. / invsqrt so it might seem backwards as fuck
  float m_inv = T(1.0f / sqrt(2.0f + 2.0f * dot));

  q_out[0] = T(0.5) / m_inv;
  q_out[1] = m_inv * (u[1] * v[2] - u[2] * v[1]);
  q_out[2] = m_inv * (u[2] * v[0] - u[0] * v[2]);
  q_out[3] = m_inv * (u[0] * v[1] - u[1] * v[0]);
}

/**
 *
 *
 * Create a quaternion from an angle and the axis. The
 *
 * `theta` Angle in radians
 * `v`     An array holding a unit 3-vector serving as the axis of rotation
 * `q`     An array holding a unit quaternion with (w, x, y, z)
 *
 *
 */
template <class T>
inline void q_from_v_theta(const T v[], const float theta, T q_out[]){

/*
  * We are using the half-angle formulas to only do one trig operation
  *
  * sin(t/2) = (-1)^(x/2pi) sqrt(0.5(1-cos(x)))
  * cos(t/2) = (-1)^(x+pi/2pi) sqrt(0.5(1-cos(x)))
  */
  float half_cos_theta = 0.5f * cos(theta);
  float cos_half_theta = std::sqrt(0.5 + half_cos_theta);
  float sin_half_theta = std::sqrt(0.5 - half_cos_theta);

  if(theta < 0.0){
    sin_half_theta = -sin_half_theta;
  }

  // This is normalized because
  //
  // len = cos^2 + sin^2 (v_i^2)
  //
  //  v is normalized and sin^2 + cos^2 = 1
  //
  q_out[0] = cos_half_theta;
  q_out[1] = sin_half_theta * v[0];
  q_out[2] = sin_half_theta * v[1];
  q_out[3] = sin_half_theta * v[2];
}

/**
 *
 *
 * Create a quaternion from an angle and an axis. The
 *
 * `theta` Angle in radians
 * `x`, `y`, `z`  An array holding a unit 3-vector serving as the axis of rotation
 * `q`     An array holding a unit quaternion with (w, x, y, z)
 *
 *
 */
template <class T>
inline void q_from_theta_xyz(T theta, T x, T y, T z, T q_out[]){

/*
  * We are using the half-angle formulas to only do one trig operation
  *
  * sin(t/2) = (-1)^(x/2pi) sqrt(0.5(1-cos(x)))
  * cos(t/2) = (-1)^(x+pi/2pi) sqrt(0.5(1-cos(x)))
  */
  float half_cos_theta = 0.5f * cos(theta);
  float cos_half_theta = sqrt(0.5 + half_cos_theta);
  float sin_half_theta = sqrt(0.5 - half_cos_theta);

  if(theta < 0.0){
    sin_half_theta = -sin_half_theta;
  }

  // This is normalized because
  //
  // len = cos^2 + sin^2 (v_i^2)
  //
  //  v is normalized and sin^2 + cos^2 = 1
  //
  q_out[0] = cos_half_theta;
  q_out[1] = sin_half_theta * x;
  q_out[2] = sin_half_theta * y;
  q_out[3] = sin_half_theta * z;
}

/**
 *
 * Multiply q_a and q_b and put it in q_out
 *
 */
template <class T>
inline void q_multi(const T q_a[], const T q_b[], T q_out[]){

  // We compute the result in a temporary to prevent aliasing of the data. when trying
  // to accumulate into a single quaternion
  float w = q_a[0] * q_b[0] - q_a[1] * q_b[1] - q_a[2] * q_b[2] - q_a[3] * q_b[3];
  float x = q_a[0] * q_b[1] + q_a[1] * q_b[0] + q_a[2] * q_b[3] - q_a[3] * q_b[2];
  float y = q_a[0] * q_b[2] + q_a[2] * q_b[0] + q_a[3] * q_b[1] - q_a[1] * q_b[3];
  float z = q_a[0] * q_b[3] + q_a[3] * q_b[0] + q_a[1] * q_b[2] - q_a[2] * q_b[1];

  q_out[0] = w;
  q_out[1] = x;
  q_out[2] = y;
  q_out[3] = z;
}


template <class T>
inline void q_to_matrix(const T q, T m_out[]){

  T q_sq[3];

  for(int i = 0; i < 3; ++i){
    q_sq[i] = q[i + 1] * q[i + 1];
  }

  for(int i = 0; i < 16; ++i){
    m_out[i] = 0.0f;
  }

  // Diagonal
  m_out[0] = 1 - 2 * q_sq[1] - 2 * q_sq[2];
  m_out[5] = 1 - 2 * q_sq[0] - 2 * q_sq[2];
  m_out[10] = 1 - 2 * q_sq[0] - 2 * q_sq[1];

  // Lower left triangle
  m_out[1] = 2. * (q[1] * q[2] + q[0] * q[3]);
  m_out[2] = 2. * (q[1] * q[3] - q[0] * q[2]);
  m_out[6] = 2. * (q[2] * q[3] + q[0] * q[1]);

  // Upper right triangle
  m_out[4] = 2. * (q[1] * q[2] - q[0] * q[3]);
  m_out[8] = 2. * (q[1] * q[3] + q[0] * q[2]);
  m_out[9] = 2. * (q[2] * q[3] - q[0] * q[1]);
}

/**
 *
 * Rotate a 3 vector by the quternion q
 *
 * p' = q p q_conj
 *
 */
template <class T>
inline void rotate_v_by_q(const T q[], const T v_in[], T v_out[]){

  using namespace std;

  float temp[4];

  // q * p
  temp[0] = -(q[1] * v_in[0] + q[2] * v_in[1] + q[3] * v_in[2]);
  temp[1] = q[0] * v_in[0] + q[2] * v_in[2] - q[3] * v_in[1];
  temp[2] = q[0] * v_in[1] + q[3] * v_in[0] - q[1] * v_in[2];
  temp[3] = q[0] * v_in[2] + q[1] * v_in[1] - q[2] * v_in[0];

//  cout<<"temp "<<temp[0]<<", "<<temp[1]<<", "<<temp[2]<<", "<<temp[3]<<endl;

  // temp * q_b
  // (qp) * q^-1
  // We compute the result in a temporary to prevent aliasing of the data. when trying
  // to accumulate into a single quaternion
//  float w = -temp[0] * q[0] + temp[1] * q[1] + temp[2] * q[2] + temp[3] * q[3];
  float x = -temp[0] * q[1] + temp[1] * q[0] - temp[2] * q[3] + temp[3] * q[2];
  float y = -temp[0] * q[2] + temp[2] * q[0] - temp[3] * q[1] + temp[1] * q[3];
  float z = -temp[0] * q[3] + temp[3] * q[0] - temp[1] * q[2] + temp[2] * q[1];

//  cout<<"xyz "<<x<<", "<<y<<", "<<z<<endl;

  v_out[0] = x;
  v_out[1] = y;
  v_out[2] = z;

//  v_out[0] = w; v_out[1] = x; v_out[2] = y; v_out[3] = z;
}


/**
 * Basic Camera managment struct with limited arcball support
 */
template <class T>
struct Camera{

  // Camera parameters
  T fov;
  T focal_length;
  T aspect;
  T near;
  T far;

  // The Projection, ModelView, MVP, Inv ModelView matrices
  T p[16];
  T mv[16];
  T mvp[16];
  T inv_p[16];
  T inv_mv[16];
  T inv_mvp[16];

  // Eye In Camera and World coordinates
  T eye[4];

  // Camera coordinate system in world coordinates
  T eye_world[4];
  T right_world[4];
  T up_world[4];

  // Default CS
  float eye_default_[4] = {0.0, 0.0, -1.0, 0.0};
  float up_default_[4] = {0.0, 1.0, 0.0, 0.0};
  float right_default_[4] = {1.0, 0.0, 0.0, 0.0};


  T eye_dist;

  // This is the direction the eye is looking at (fwd direction)
  T eye_orientation[3];

  int width_;
  int height_;

  bool orthographic_ = false;

  Camera() {

  }

  Camera(T fov, T near, T far, int width, int height)
      : fov(fov), near(near), far(far),
        width_(width), height_(height) {

    aspect = T(width) / T(height);
    focal_length = Camera::init_perspective(fov, aspect, near, far, p);
    init_model_view();
  }

  /*
   * From:
   *    http://www.songho.ca/opengl/gl_projectionmatrix.html
   *
   * Construct projecton matrix in <<Row-Major>> order as expected by OpenGL
   *
   */
  void init_orthographic(int width, int height, T left, T top, T near, T far) {
    aspect = T(width) / T(height);
    width_ = width;
    height_ = height;

    this->near = near;
    this->far = far;

    this->fov = fov;

    init_orthographic(left, -left, top, -top, near, far, p);


    this->orthographic_ = true;
  }

  /**
   *
   * Initialize an orthographic matrix given by p.
   *
   *  More Info: http://www.songho.ca/opengl/gl_projectionmatrix.html#ortho
   *
   * @param l   Left
   * @param r   Right
   * @param t   Top
   * @param b   Bottom
   * @param n   Near
   * @param f   Far
   *
   * @return
   */
  static void init_orthographic(const T l, const T r,
                                const T b, const T t,
                                const T n, const T f, float p[]) {


    T inv_r_m_l = T(1) / (r - l);
    T inv_t_m_b = T(1) / (t - b);
    T inv_n_m_f = T(1) / ( n - f);

    using namespace std;
#if 0
    cout << "camera::init_orthographic "
         << ", left = " << l << ", right = " << r
         << ", top = " << t << ", bottom = " << b
         << ", near = " << n << ", far = " << f << endl;
#endif
    // Column 0
    p[0] = T(2.0 * inv_r_m_l);
    p[1] = 0.0;
    p[2] = 0.0;
    p[3] = 0.0;

    // Column 1
    p[4] = 0.0;
    p[5] = T(2.0 * inv_t_m_b);
    p[6] = 0.0;
    p[7] = 0.0;

    // Column 2
    p[8] = 0.0;
    p[9] = 0.0;
    p[10] = T(2.0 * inv_n_m_f);
    p[11] = 0.0;

    // Column 3
    p[12] = (r + l) * inv_r_m_l;
    p[13] = (t + b) * inv_r_m_l;
    p[14] = (f + n) * inv_n_m_f;
    p[15] = T(1.0);
  }

  /*
   * From:
   * http://www.songho.ca/opengl/gl_projectionmatrix.html
   *
   * Construct projecton matrix in <<Row-Major>> order as expected by OpenGL
   *
   */
  void init_perspective(int width, int height, T fov, T near, T far) {
    aspect = T(width) / T(height);
    width_ = width;
    height_ = height;

    this->fov = fov;
    this->near = near;
    this->far = far;

    focal_length = Camera::init_perspective(fov, aspect, near, far, p);

    // TODO: Do we need this?
    invert_4x4(p, inv_p);
    orthographic_ = false;
  }

  /**
   *
   * Initialize a Perspective matrix given by p
   *
   * @param fov
   * @param aspect
   * @param near
   * @param far
   * @param p
   * @return
   */
  static T init_perspective(const T fov, const T aspect, const T near, const T far, float p[]) {

    using namespace std;

//    cout << "camera::init_perspective fov = " << fov << ", aspect = " << aspect
//         << ", near = " << near << ", far = " << far << endl;

    T focal_length = T(1.0 / tan(fov * M_PI / 360.0));
    T fpn = near + far;
    T inv_fmn = T(1) / (near - far);

    p[0] = focal_length / aspect;
    p[1] = 0.0;
    p[2] = 0.0;
    p[3] = 0.0;

    p[4] = 0.0;
    p[5] = focal_length;
    p[6] = 0.0;
    p[7] = 0.0;

    p[8] = 0.0;
    p[9] = 0.0;
    p[10] = fpn * inv_fmn;
    p[11] = -1.0;

    // Column 3
    p[12] = 0.0;
    p[13] = 0.0;
    p[14] = T(2.0f * near * far * inv_fmn);
    p[15] = 0.0;

    return focal_length;
  }

  /**
   *
   * Construct a default MV matrix
   *
   */
  void init_model_view(){
    // Zero out MV
    for(int i = 0; i < 16; ++i){
      mv[i] = 0.0;
    }

    // Rotation
    mv[0] = 1.0;
    mv[5] = 1.0;
    mv[10] = 1.0;

    // Translation
//    mv[14] = -2.5;
    mv[14] = -2.5;
    mv[15] = 1.0;

    invert_4x4(mv, inv_mv);
  }

  /**
   *
   * @param center
   * @param eye
   * @param up
   */
  void look_at(const T center[3], const T eye[3], T up[3]) {
    using namespace std;

    // Clear model-view matrix
    for(int i = 0; i < 16; ++i){
      mv[i] = T(0);
    }

    // This is basically glLookAt
    // https://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml
    //  or
    // https://www.opengl.org/wiki/GluLookAt_code
    //
    // Load center, eye, up into mv

    T fwd[3];
    T side[3];

    T eye_dist_sq = T(0);

    for(int i = 0; i < 3; ++i){
      this->eye[i] = eye[i];
      fwd[i] = center[i] - eye[i];
      eye_dist_sq += (fwd[i] * fwd[i]);
    }

    eye_dist = sqrt(eye_dist_sq);
    T eye_dist_inv = T(1) / eye_dist;

    for(int i = 0; i < 3; ++i){
      fwd[i] *= eye_dist_inv;
      eye_orientation[i] = fwd[i];
    }

    side[0] = fwd[1] * up[2] -  up[1] * fwd[2];
    side[1] = fwd[2] * up[0] - fwd[0] *  up[2];
    side[2] = fwd[0] * up[1] -  up[0] * fwd[1];

    up[0] = side[1] * fwd[2] - side[2] * fwd[1];
    up[1] = fwd[0] * side[2] - side[0] * fwd[2];
    up[2] = side[0] * fwd[1] - side[1] * fwd[0];

    for(int i = 0; i < 3; ++i){
      mv[4*i] = side[i];
      mv[1 + 4*i] = up[i];
      mv[2 + 4*i] = -fwd[i];
    }

    for(int i = 0; i < 4; ++i) {
      mv[12 + i] = -(mv[i] * eye[0] + mv[4 + i] * eye[1] + mv[8 + i] * eye[2]);
    }
    mv[15] = T(1);

    invert_4x4(mv, inv_mv);

    // Init fov, clipping
    generate_transform();
  }

  /**
   *
   *  Initialize with a quaternion and translation
   *
   *  q - 4-array holding a quaternion in (w, x, y, z)
   *  r - 3-array holding a 3-vector in (x,y,z)
   *
   */
  void init_from_quaternion(const float q[], const float r[]){

    // Clear model-view matrix
    for(int i = 0; i < 16; ++i){
      mv[i] = 0.0;
    }

    T q_sq[3];

    for(int i = 0; i < 3; ++i){
      q_sq[i] = q[i+1] * q[i+1];
      eye[i] = -r[i];
    }

    // Diagonal
    mv[0] = 1 - 2 * q_sq[1] - 2 * q_sq[2];
    mv[5] = 1 - 2 * q_sq[0] - 2 * q_sq[2];
    mv[10] = 1 - 2 * q_sq[0] - 2 * q_sq[1];

    // Lower left triangle
    mv[1] = 2. * (q[1] * q[2] + q[0] * q[3]);
    mv[2] = 2. * (q[1] * q[3] - q[0] * q[2]);
    mv[6] = 2. * (q[2] * q[3] + q[0] * q[1]);

    // Upper right triangle
    mv[4] = 2. * (q[1] * q[2] - q[0] * q[3]);
    mv[8] = 2. * (q[1] * q[3] + q[0] * q[2]);
    mv[9] = 2. * (q[2] * q[3] - q[0] * q[1]);


    for(int i = 0; i < 3; ++i){
      mv[12 + i] = r[i];
    }
    mv[15] = 1.0f;

//    std::cout<<"Quaternion Matrix "<<std::endl;
//    print_4x4(mv);

    // TODO: Can compute this faster, directly from quat above.
    invert_4x4(mv, inv_mv);

    generate_transform();
  }

  /**
   * Camera::generate_transform
   */
  void generate_transform(){

    Camera<T>::generate_transform(p, mv, mvp);

    // Invert the MV and MVP matrices so we can unproject stuff
    // TODO: This is not efficient
    invert_4x4(mvp, inv_mvp);

    // Compute the Eye, Up, Right coordinates in World Space
    vec4_by_mat4x4(inv_mv, eye_default_, eye_world);
    vec4_by_mat4x4(inv_mv, right_default_, right_world);
    vec4_by_mat4x4(inv_mv, up_default_, up_world);
  }


  /**
   * Camera::generate_transform
   *
   * @param p Projection matrix
   * @param mv Model View matrix
   */
  static void generate_transform(const T p[], const T mv[], T mvp[]){
//    std::cout<<"camera::generate_transform"<<std::endl;

    // Only non-zero elements of the projection
    // matrix are

    // Perspective
    // p[0], p[5], p[10], p[11], p[14]

    // Orthographic
    // p[0], p[5], p[10],   p[12], p[13], p[14], p[15]
    //
    // Model View is dense for the first 3 rows an row 4 is (0 0 0 1)

    T p1 = p[0];
    T p2 = p[5];
    T p3 = p[10];
    T p4 = p[14];
    T p5 = p[11];

    /*
     * This looks weird but it's just a specialization of matrix multiplication
     * where one matrix, p, is nearly diagonal.
     *
     *       P               M V
     * [  u   0   0   n]  [a b c alpha ]
     * [  0   v   0   m]  [d e f beta  ]
     * [  0   0   w   k]  [g h i gamma ]
     * [  0   0   j   q]  [0 0 0   1   ]
     *
     * M - V - P =
     *
     * [ a * u     b * u   c * u    u * alpha + n ]
     * [ d * v     e * v   f * v    v * beta + m  ]
     * [ g * w     h * w   i * w    w * gamma + k ]
     * [ g * j     h * j   i * j    j * gamma + q ]
     *
     */

    int offset = 0;

#if 1
    for(int i = 0; i < 3; ++i){
      mvp[offset + 0] = p1 * mv[offset];
      mvp[offset + 1] = p2 * mv[offset + 1];
      mvp[offset + 2] = p3 * mv[offset + 2] + p4 * mv[offset + 3];
      mvp[offset + 3] = p5 * mv[offset + 2];
      offset += 4;
    }

//    std::cout<<"Creating MVP "<<offset<<std::endl;
    for(int i = 0; i < 3; ++i, ++offset) {
      mvp[offset] = p[5 * i] * mv[offset] + p[12 + i];
    }

    mvp[offset] = p[11] * mv[14] + p[15];

//    std::cout<<"offset "<<offset<<std::endl;
//    std::cout<<"DDDD mvp[offset] "<<mvp[offset]<<std::endl;
//    std::cout<<"DDDD mv[14] "<<mv[14]<<std::endl;
//    std::cout<<"DDDD p[11] "<<p[11]<<std::endl;
//    std::cout<<"DDDD p[15] "<<p[15]<<std::endl;
#else
    for(int i = 0; i < 4; ++i){
      mvp[offset + 0] = p1 * mv[offset];
      mvp[offset + 1] = p2 * mv[offset + 1];
      mvp[offset + 2] = p3 * mv[offset + 2] + p4 * mv[offset + 3];
      mvp[offset + 3] = p5 * mv[offset + 2];

      offset += 4;
    }
#endif
  }

  /**
   *
   * Generate an arcball vector
   *
   */
  void arcballVector(float x, float y, float v_out[]){

    float radius = 1.0f;

    float v_ndc[4];

    v_ndc[0] = -(x - 0.5f * width_) / (0.5f * radius * width_);
    v_ndc[1] = (y - 0.5f * height_) / (0.5f * radius * height_);
    v_ndc[2] = 0.0f;
    v_ndc[3] = 1.0f;

    float p_mag = v_ndc[0] * v_ndc[0] + v_ndc[1] * v_ndc[1];

    // Hyperbolic sheet
//    float half_r_sq = 0.5 * radius * radius;
//    if(p_mag < half_r_sq){
//      v_ndc[2] = -sqrt(max(0.0f, radius*radius - p_mag));
//    }else{
//      v_ndc[2] = -half_r_sq / sqrt(p_mag);
//    }

    // Plane thing
    if(p_mag > 1.0){
      float scale = 1.0f / std::sqrt(p_mag);
      v_ndc[0] *= scale;
      v_ndc[1] *= scale;
    }else{
      v_ndc[2] = - std::sqrt(std::max(0.0f, radius*radius - p_mag));
    }

    // Transform to World-Space
    vec4_by_mat4x4(inv_mv, v_ndc, v_out);
  }

  /**
   *
   * Unproject the window coordinates given by x, y,
   * into
   *
   * @param x
   * @param y
   * @param z_ndc The normalized z component to use -1 is front, 1.0 is back)
   * @param world
   */
  void unproject(float x, float y, float z_ndc, float world[]){

    float inv_width = 1.0f / width_;
    float inv_height = 1.0f / height_;

    float ndc_cs[4] = {-1.0f + 2.0f * float(x) * inv_width,
                       -1.0f + 2.0f * float(height_ - y) * inv_height, z_ndc, 1.0f};

    float world_cs[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    vec4_by_mat4x4(inv_mvp, ndc_cs, world_cs);

    float scale = orthographic_ ? 1.0f : eye_dist;

    for(int i = 0; i < 3; ++i){
      world[i] = world_cs[i] * scale;
    }
  }
};

namespace gl_consts{

/**
 * The anti-alliasing texture for the star faces
 *
 * http://translate.google.de/translate?js=n&prev=_t&hl=de&ie=UTF-8&layout=2&eotf=1&sl=de&tl=en&u=http://zfx.info/viewtopic.php%3Ff%3D11%26t%3D8
 *
 *
 */
  const uint8_t aaData_[] = {255, 253, 250, 246, 241, 234, 226, 216, 203, 189, 173, 156, 138, 120,
                             102, 85, 70, 56, 43, 32, 24, 18, 14, 11, 8, 6, 4, 3, 2, 1, 0, 0};

}

#endif // HPP_NEWBERRY_GL_UTIL
