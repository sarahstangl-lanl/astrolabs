/**
 *
 * msg.h - Micro Scene Graph
 *
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu> on
 * Created: 10 Oct 2016
 * License: Apache 2.0
 *
 *
 * Lightweight `scene graph` library for OpenGL based graphics... this is meant as
 * a replacement for OpenSceneGraph for basic use cases. Point is also to be clear
 * and easy to hack on. There are no generic wrappers for OpenGL objects and the
 * hierarchies are kept as flat as possible. This has only the features needed
 * by the astro programs.
 *
 * This scene graph will not hold your code hostage and relies on OpenGL Core Profile
 *
 */

#ifndef ASTROLABS_SCENE_GRAPH_H
#define ASTROLABS_SCENE_GRAPH_H

#include <list>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>

#include "gl_util.hpp"

// Image processing
#include <QImage>
#include <QImageReader>
#include <QPainter>

namespace msg{

  /**
   *
   * Parent class for all MSG Nodes.
   *
   */
  struct Node{

    // Transformation
    float position[4] = {0, 0, 0, 0};

    // Scaling (TODO:)
    float scale[3] = {1.0f, 1.0f, 1.0f};

    /**
     * Drawing Internals
     */

    void setPosition(float x, float y, float z){
      position[0] = x;
      position[1] = y;
      position[2] = z;
    }

    Node(){

    }

    /**
     *
     */
    virtual void cleanup() = 0;

    virtual bool hitTest(float x, float y){
      return false;
    };

    virtual bool handleDrag(float x_i, float y_i, float x_f, float y_f){
      return false;
    };

    // Update child node transforms if any
    virtual void updateChildren(){
    }

    /**
     *
     * Render the current node however you want.
     *
     */
    virtual void render(Camera<float>& camera) = 0;

  };


  /**
   *
   * A single user-facing billboard. This should be
   * used for complicated Billboards with custom
   * programs (e.g. Planets)
   *
   *
   */
  struct Billboard : public Node{

    float radius = 0.0;
    float inclination_ = 0.0f;

    GLuint vao = 0;
    GLuint vbo = 0;

    float vertices_[3][3];

    Billboard(){

    }

    // http://stackoverflow.com/questions/21652546/what-is-the-role-of-glbindvertexarrays-vs-glbindbuffer-and-what-is-their-relatio
    virtual bool init_resources(){

      // Create the VBO and VAO indices
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

//      radius = 1.0;

      float half_a = std::sqrt(3.0) * radius;

      const float vertices[3][3]{{-half_a, -radius,      0.0f},
                                 {0.0f,   2.0f * radius, 0.0f},
                                 {half_a,  -radius,      0.0f}};

      // Create the VAO
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glBindVertexArray(0);

      return check_GL_error("Sphere::init_resources() exit");
    }

    virtual void cleanup(){
      glDeleteBuffers(1, &vbo);
      glDeleteVertexArrays(1, &vao);
    }

    /**
     *
     * Billboard::setup_array
     *
     * @param attributeIndex
     */
    void setup_array(int attributeIndex){

 //     std::cout<<"Attribute Index "<<attributeIndex<<std::endl;

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(attributeIndex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      glEnableVertexAttribArray(attributeIndex);

      check_GL_error("Billboard::init_attribute()");
    }

    virtual void render(Camera<float>& camera){

//      std::cout<<"Sphere::render() "<<radius<<std::endl;
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);

//    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, const_ix);
    }
  };

  /**
   *
   *  This represents a flat shape, either a single triangle or a quad
   *
   *  TODO: Another way to store this is we have a geometry generator class that loads data arrays
   *  with whatever shape you want.  Maybe the template argument can
   */
  struct FlatShape : public Node{

    const unsigned int const_ix[6] = {0, 1, 2, 2, 3, 1};

    GLuint vao = 0;
    GLuint vbo = 0;

    int count_ = 0;

    FlatShape(){

    }

    bool init_resources(){
      // Create the VBO and VAO indices
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);
      return true;
    }

    void cleanup(){
      glDeleteBuffers(1, &vbo);
      glDeleteVertexArrays(1, &vao);
    }

    /**
     *
     * Load the geometry for a quad into array `data`
     *
     * @param width
     * @param height
     * @param data
     */
    static int generate_quad(float width, float height, float data[]){

      const float vertices[4][3]{{-0.5f * width,  0.5f * height, 0.0f}, {0.5f * width,  0.5f * height, 0.0f},
                                 {-0.5f * width, -0.5f * height, 0.0f}, {0.5f * width, -0.5f * height, 0.0f}};

      int offset = 0;
      for(int j = 0; j < 4; ++j){
        for(int i = 0; i < 3; ++i) {
          data[offset++] = vertices[j][i];
        }
      }

      return offset;
    }

    void build_quad(float width, float height){

      count_ = 6;

      float data[12];

      generate_quad(width, height, data);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

      glBindVertexArray(0);
    }

    void build_triangle(float radius){

      count_ = 3;

      float half_a = sqrt(3.0) * radius;

      const float vertices[3][3]{{-half_a, -radius,      0.0f},
                                 {0.0f,   2.0f * radius, 0.0f},
                                 {half_a,  -2 * radius,      0.0f}};

      // Create the VAO
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glFinish();

      glBindVertexArray(0);
    }

    void setup_array(int positionHandle){

      std::cout<<"FlatShape::setup_array positionHandle "<<positionHandle<<std::endl;

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(positionHandle, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      glEnableVertexAttribArray(positionHandle);

      check_GL_error("FlatShape::init_attribute()");
    }

    void bind(){
      glBindVertexArray(vao);
    }

    virtual void render(Camera<float>& camera){
//      std::cout<<"FlatShape::render"<<std::endl;

//      if(!enabled_){
//        return;
//      }

      glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, const_ix);
    }

    virtual bool hitTest(float x, float y){
      using namespace std;

      std::cout<<"FlatShape::hitTest : x= "<<x<<", y= "<<y<<endl;

      return false;
    };

    virtual bool handleDrag(float x_i, float y_i, float x_f, float y_f){
      return false;
    };


  };

  /**
   *
   * A line path
   *
   */
  struct Path : public Node{

    GLuint vao = 0;
    GLuint vbo = 0;

    // Maximum number of elements
    int capacity_ = 0;

    const static int stride_ = 3;

    // Vertices for the path
    float *data_ = nullptr;

    float *velocity_ = nullptr;

    // How many elements are in the path now
    int size = 0;

    // How many should be drawn
    int draw_size = 0;

    Path(int capacity) : capacity_(capacity){
      init(capacity);
    }

    Path(){

    }

    void init(int capacity){

      capacity_ = capacity;
      data_ = new float[stride_ * capacity_];
      velocity_ = new float[stride_ * capacity_];

      for(int i = 0; i < stride_ * capacity_; ++i){
        data_[i] = 0.0f;
        velocity_[i] = 0.0f;
      }
    }


    ~Path(){
      delete[] data_;
      delete[] velocity_;
    }

    void clear(){
      size = 0;
      draw_size = 0;
    }

    /**
     * Path::init_resources
     *
     *   It allocates storage and uploads whatever is in data_
     *
     * @return
     */
    virtual bool init_resources(){

      // Create the VAO and the buffers
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

      // Initialize the storage (uplad
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, stride_ * capacity_ * sizeof(float), data_, GL_STATIC_DRAW);
      return check_GL_error("path::init_resources()");
    }

    /**
     * Path::setup_array
     *
     * @param positionHandle
     * @param colorHandle
     */
    void setup_array(int positionHandle, int colorHandle){
#if 0
      std::cout<<"positionIndex "<<positionHandle<<" colorIndex "<<colorHandle<<std::endl;
#endif
      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);

      glVertexAttribPointer(positionHandle, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(float), nullptr);
      glEnableVertexAttribArray(positionHandle);

//      glVertexAttribPointer(colorHandle , 3, GL_FLOAT, GL_FALSE, stride_ * sizeof(float), (void *) (3 * sizeof(float)));
//      glEnableVertexAttribArray(colorHandle);

      check_GL_error("path::init_attribute()");
    }

    void bind(){
      glBindVertexArray(vao);
    }

    /**
     * Path::addPoint
     *
     * After adding the points you still need to specify draw_size;
     *
     * @param x
     * @param y
     * @param z
     * @param v_x
     * @param v_y
     * @param v_z
     * @return
     */
    bool addPoint(float x, float y, float z,
                  float v_x, float v_y, float v_z){

      using namespace std;

//      std::cout<<" Path Vertices "<<x<<", "<<y<<", "<<z<<endl;

      int offset = stride_ * size;

      if(size >= capacity_){
        cerr<<"Path::addPoint:: Not enough space"<<endl;
        return false;
      }

      // Position Data
      data_[offset] = x;
      data_[offset + 1] = y;
      data_[offset + 2] = z;

      // Velocity
      velocity_[offset] = v_x;
      velocity_[offset + 1] = v_y;
      velocity_[offset + 2] = v_z;

      size += 1;

      return true;
    }

    /**
     *
     *
     *
     *
     * @param t from 0 to 1.0
     * @param position[] A 3-array with the interpolated position
     */
    int interpolate(const float t, float* position){

      int ix = t * size;
      int ix_next = (ix + 1) % size;

      // TODO: Linearly interpolate between the two points

      std::cout<<"interpolate t="<<t<<", ix = "<<ix<<", ix_next = "<<ix_next<<std::endl;

      int offset = 3 * ix;
//      int offset_next = 3 * ix_next;

      for(int i = 0; i < 3; ++i){
        position[i] = data_[offset + i];
      }

      return ix;
    }

    /**
     * Path::upload
     *
     * Upload the data
     *
     */
    void upload(){
      check_GL_error("path::upload() - entry");

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
//      check_GL_error("path::upload() - entry 1");

//      std::cout<<"Path::upload Uploading "<<stride_ * size * sizeof(float) <<" bytes"<<std::endl;

      glBufferSubData(GL_ARRAY_BUFFER, 0, stride_ * size * sizeof(float), data_);
      check_GL_error("path::upload() - exit");
    }

    /**
     * Draw the path on screen
     */
    virtual void render(Camera<float>& camera_){
      using namespace std;

//      cout<<"Path::render "<<draw_size<<endl;
      glBindVertexArray(vao);
      glDrawArrays(GL_LINE_STRIP, 0, draw_size);
      check_GL_error("path::render() - exit");
    }

    /**
     *
     * Do a line - cylinder intersection
     *
     *
     * @param x Screen coordinates in NDC (?)
     * @param y
     * @param radius size of pick region
     * @param element_ix [out] The index of the hit vertex
     *
     * @return square of the distance to the closest point
     */
    float hitTest(float x, float y, float radius, const Camera<float>& camera_,
                  int *element_ix, float element_position[],
                  float element_velocity[]) {

      // TODO: This needs to interpolate between the nearest hits

      float vert_world[4] {0.0f, 0.0f, 0.0f, 1.0f};
      float vert_screen[4] {0.0f, 0.0f, 0.0f, 1.0f};

      // We want the closest point to x, y so set the target radius as the initial threshold
      // and with each hit we set the threshold to the new dr_sq since it will be smaller.
      float r_sq_min = radius * radius;

      // Go through all of the points
      int offset = 0;

      *element_ix = -1;

      for (int j = 0; j < size; ++j){

        for(int i = 0; i < 3; ++i){
          vert_world[i] = data_[offset + i];
        }

        vec4_by_mat4x4(camera_.mvp, vert_world, vert_screen);

        float scale = 1.0f / vert_screen[3];
        vert_screen[0] *= scale;
        vert_screen[1] *= scale;

        float dx = vert_screen[0] - x;
        float dy = vert_screen[1] - y;
        float dr_sq = dx * dx + dy * dy;

        if(dr_sq < r_sq_min){
          r_sq_min = dr_sq;
          *element_ix = j;

          // We need the hit location
          for(int i = 0; i < 3; ++i){
            element_position[i] = vert_world[i];
            element_velocity[i] = velocity_[offset + i];
          }

        }

        offset += 3;

#if 0
        if(j % 10 == 0){
          std::cout << "Path::hitTest: j " << j << " " << vert_screen[0] << ", " << vert_screen[1]

                    <<" dx "<<dx<<", dy "<<dy<<" dr_sq "<<dr_sq
                    << std::endl;
        }
      }

      if(*element_ix >= 0){
        std::cout<<"Path::hitTest Picking at "<<x<<", "<<y<<" resulted in pick " <<*element_ix<< std::endl;
      }
#else
      }
#endif

      return r_sq_min;
    }

    /**
     * Free OpenGL resources
     */
    void cleanup(){
      glDeleteBuffers(1, &vbo);
      glDeleteVertexArrays(1, &vao);
    }
  };

  /**
   *
   * A set of Sprites
   *
   */
  struct Sprites : public Node{

    GLuint vao = 0;
    GLuint vbo = 0;

    // Maximum number of elements
    int capacity_ = 0;
    const static int stride_ = 8;

    // Vertices for the path
    float *data_;

    // How many points do we have
    int size = 0;

    // How many are we drawing.
    int draw_size = 0;

    Sprites(int capacity) : capacity_(capacity){

      data_ = new float[stride_ * capacity_];

      for(int i = 0; i < stride_ * capacity_; ++i){
        data_[i] = 0.0f;
      }
    }

    ~Sprites(){
      delete[] data_;
    }

    /**
     * Sprites::init_resources
     *
     * @return
     */
    virtual bool init_resources(){
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, stride_ * capacity_ * sizeof(float), 0, GL_STATIC_DRAW);

      return check_GL_error("Path::init_resources() - exit");
    }

    void clear(){
      size = draw_size = 0;
    }

    /**
     * Sprites::setup_array
     *
     * @return
     */
    void setup_array(int positionHandle, int sizeHandle, int colorHandle){

      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);

      glVertexAttribPointer(positionHandle, 3, GL_FLOAT, GL_FALSE, stride_ * sizeof(float), (void *) 0);

      glEnableVertexAttribArray(positionHandle);

      // We've packed the normalized Temperature and normalized Irradiance into one 2 component attribute
      glVertexAttribPointer(sizeHandle, 1, GL_FLOAT, GL_FALSE, stride_ * sizeof(float), (void *) (3 * sizeof(float)));
      glEnableVertexAttribArray(sizeHandle);

      // We've packed the normalized Temperature and normalized Irradiance into one 2 component attribute
      glVertexAttribPointer(colorHandle , 3, GL_FLOAT, GL_FALSE, stride_ * sizeof(float), (void *) (4 * sizeof(float)));
      glEnableVertexAttribArray(colorHandle);

      check_GL_error("Sprites::init_attribute() 4");
    }

    /**
     * Sprites::bind
     *
     * @return
     */
    void bind(){
      glBindVertexArray(vao);
    }

    /**
     * Sprites::addPoint
     *
     * @param radius Size of pixel
     * @param r Color
     *
     * @return
     */
    bool addPoint(float x, float y, float z, float radius,
                  float r, float g, float b, float a){

      using namespace std;

#if 0
     std::cout<<" Sprites::addPoint Vertices "<<x<<", "<<y<<", "<<z
              <<", "<<r<<", "<<g<<", "<<b<<", "<<a<<endl;
#endif
      int offset = stride_ * size;

      if(size >= capacity_ ){
        cerr<<"Sprites::addPoint: Not enough space to add vertex. capacity = "<<capacity_<<endl;
        return false;
      }

      data_[offset] = x;
      data_[offset + 1] = y;
      data_[offset + 2] = z;

      data_[offset + 3] = radius;

      data_[offset + 4] = r;
      data_[offset + 5] = g;
      data_[offset + 6] = b;
      data_[offset + 7] = a;

      size += 1;

      return true;
    }

    /**
     * Sprites::upload
     *
     * Upload the data
     */
    void upload(){

      check_GL_error("path::upload() - entry");
//      std::cout<<"Sprites::addPoint Uploading Points "<<stride_ * size<<std::endl;

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, stride_ * size * sizeof(float), data_);

      check_GL_error("path::upload() - exit");
    }

    /**
     * Draw the path on screen
     */
    virtual void render(Camera<float>& camera_){
#if 0
      std::cout<<"Sprites::render: Drawing points "<<draw_size<<" of "<<size<<std::endl;
#endif
      glBindVertexArray(vao);
      glDrawArrays(GL_POINTS, 0, draw_size);
    }

    /**
     * Free OpenGL resources
     */
    void cleanup(){
      glDeleteBuffers(1, &vbo);
      glDeleteVertexArrays(1, &vao);
    }

  };


  /**
   *
   *  A generic geometry class
   *
   */
  struct Geometry : public Node{

    // Shaders
    std::string vert_shader = "./assets/shaders/default.vert";
    std::string frag_shader = "./assets/shaders/default.frag";

    //
    GLenum mode = GL_LINES;

    // Geometry Vertex Array and Vertex Buffer
    GLuint vao = 0;
    GLuint vbo = 0;

    // Program handle
    GLuint program_;

    // Position Attribute
    GLint positionHandle_;
    GLint colorHandle_;

    GLint mvpHandle_;
    GLint solidColorHandle_;

    /**
     * Data
     */

    float defaultColor_[4] = {1.0f, 0.0f, 1.0f, 1.0f};

    float *verts_;

    // Number of vertices
    int count_ = 0;

    int capacity_ = 0;

    // Size of array in elements (attributes_per_vertex_ * line_count)
    int size_ = 0;

    // 3-position and 3-color
    const static int stride_ = 7;

    Geometry(int line_count)
        : count_(0),
          capacity_(2 * line_count),
          size_(2 * stride_ * line_count){

      std::cout<<"Creating Geometry object with size "<<size_<<std::endl;

      verts_ = new float[size_];

      for(int i = 0; i < size_; ++i){
        verts_[i] = 0.0f;
      }
    }

    ~Geometry(){
      delete[] verts_;
    }

    /**
     *
     * Geometry::init_resources
     *
     */
    virtual bool init_resources(){

      std::cout<<"Geometry::init_resources"<<std::endl;

      /**
       * Build programs
       */
      program_ = glCreateProgram();

      bool status = build_program(program_, "Geometry", vert_shader.c_str(), frag_shader.c_str());
      if(!status){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position_in_");
      colorHandle_ = glGetAttribLocation(program_, "color_in_");

      mvpHandle_ = glGetUniformLocation(program_, "mvp_");
      solidColorHandle_ = glGetUniformLocation(program_, "solid_color_");

      // Create the VBO and VAO indices
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, size_ * sizeof(float), verts_, GL_STATIC_DRAW);

      glVertexAttribPointer(positionHandle_, 3, GL_FLOAT, GL_FALSE, stride_ * sizeof(float), (void *) 0);
      glEnableVertexAttribArray(positionHandle_);

      if(colorHandle_ > 0){
        glVertexAttribPointer(colorHandle_, 4, GL_FLOAT, GL_FALSE, stride_ * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(colorHandle_);
      }

      glBindVertexArray(0);

      return check_GL_error("Geometry::init_resources - exit");
    }

    void clear(){
      count_ = 0;
    }

    void cleanup(){
      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
    }

    /**
     * Gometry::addPoint
     *
     * @param v A 3-array
     */
    void addPoint(float v[], float c[]){

      int offset = stride_ * count_;

      for(int i = 0; i < 3; ++i){
        verts_[offset++ ] = v[i];
      }

      for(int i = 0; i < 4; ++i){
        verts_[offset++] = c[i];
      }
      ++count_;
    }

    /**
     *
     * Geometry::upload
     *
     */
    void upload(){
#if 0
      std::cout<<"Geometry::upload "<<count_<<" lines or "
               <<2 * stride_ * count_ * sizeof(float)<<" bytes"<<std::endl;
#endif
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, stride_ * count_ * sizeof(float), verts_);

      check_GL_error("Geometry::upload exit ");
    }

    /**
     *
     * Geometry::init_resources
     *
     */
    virtual void render(Camera<float>& camera_){
//      std::cout<<"Geometry::render() "<<count_<<std::endl;

      glUseProgram(program_);
      glUniformMatrix4fv(mvpHandle_, 1, GL_FALSE, camera_.mvp);
      glUniform4fv(solidColorHandle_, 1, defaultColor_);


      glBindVertexArray(vao);
      glDrawArrays(GL_LINES, 0, count_);
    }

  };

  /**
   *  Programs
   */
  struct DefaultProgram{

    // Program handle
    GLuint program_;

    // Position Attribute
    GLuint positionHandle_;
    GLuint colorHandle_;

    // Uniform handles
    GLint mvpHandle_;

    DefaultProgram(){

    }

    /**
     * Initialize resources
     */
    bool init_resources(){
      program_ = glCreateProgram();

      return true;
    }

    void cleanup(){
      glDeleteProgram(program_);
    }

    bool build(){

      const char *vertexShaderSource =
          "#version 330\n"
              "uniform mat4 mvp_;\n"
              "in vec3 position_in_;\n"
              "in vec4 color_in_;\n"
              "out vec4 color_ex;\n"
              "void main() {\n"
              "   color_ex = color_in_;\n"
              "   gl_Position = p_ * mv_ * vec4(position_in_, 1.0);\n"
              "}\n";

      const char *fragmentShaderSource =
          "#version 330\n"
          "in vec4 color_ex;\n"
              "out vec4 color_out_;\n"
              "\n"
              "void main() {\n"
              "   color_out_ = color_ex;\n"
              "}\n";

      check_GL_error("DefaultProgram::build() exit 0 ");


      bool status = build_program_from_source(program_,
                                              "default",
                                              vertexShaderSource,
                                              fragmentShaderSource);
      if(!status){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position");
      colorHandle_ = glGetAttribLocation(program_, "position");
      mvpHandle_ = glGetUniformLocation(program_, "mvp_");

      return check_GL_error("DefaultProgram::build() exit");
    }

  };


  /**
   * Program for drawing paths
   */
  struct PathProgram{

    // Program handle
    GLuint program_;

    // Attributes
    GLuint positionHandle_;

    // Uniform handles
    GLint mvpHandle_;
    GLint colorHandle_;

    /**
     * Initialize resources
     */
    bool init_resources(){
      program_ = glCreateProgram();

      return true;
    }

    void cleanup(){
      glDeleteProgram(program_);
    }

    bool build(){

      const char *vertexShaderSource =
          "#version 330\n"
              "uniform mat4 mvp_;\n"
              "uniform vec4 color_ = vec4(1.0, 1.0, 1.0, 1.0);\n"
              "in vec3 position;\n"
              "out vec4 color_ex;\n"
              "void main() {\n"
              "   color_ex = color_;\n"
              "   gl_Position = mvp_ * vec4(position, 1.0);\n"
              "}\n";

      const char *fragmentShaderSource =
          "#version 330\n"
              "in vec4 color_ex;\n"
              "out vec4 color_out;\n"
              "\n"
              "void main() {\n"
              "   color_out = color_ex;\n"
              "}\n";



      bool status = build_program_from_source(program_, "default",
                                              vertexShaderSource,
                                              fragmentShaderSource);

      if(!status){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position");
      mvpHandle_ = glGetUniformLocation(program_, "mvp_");
      colorHandle_ = glGetUniformLocation(program_, "color_");

      return check_GL_error("PathProgram::build() exit");
    }

  };


  /**
   * Draw a sphere on a triangle
   */
  struct SphereProgram{

    // Shaders
    std::string vert_shader = "./assets/shaders/sphere.vert";
    std::string frag_shader = "./assets/shaders/sphere.frag";

    // Program handle
    GLuint program_;

    // Position Attribute
    GLuint positionHandle_;

    // Uniform handles
    GLint mvpHandle_;

    GLint colorHandle_;
    GLint offsetHandle_;
    GLint radiusHandle_;

    SphereProgram(){

    }

    /**
     * Initialize resources
     */
    bool init_resources(){
      program_ = glCreateProgram();

      return true;
    }

    void cleanup(){
      glDeleteProgram(program_);
    }

    bool build(){

#if 0
      const char *vert_source =
          "#version 330\n"
              "uniform mat4 mvp_;\n"
              "uniform vec4 solid_color_ = vec4(1.0, 0.0, 1.0, 1.0);\n"
              "uniform vec3 offset_ = vec3(0);\n"
              "uniform float radius_ = 1.0f;\n"
              "in vec3 position_;\n"
              "out vec4 color_ex;\n"
              "out vec2 coord_;\n"
              "void main() {\n"
              "   color_ex = solid_color_;\n"
              "   coord_ = position_.xy / radius_;\n"
              "   gl_Position = mvp_ * vec4(position_ + offset_, 1.0);\n"
              "}\n";

      const char *frag_source =
          "#version 330\n"
              "in vec2 coord_;\n"
              "in vec4 color_ex;\n"
              "out vec4 color_out;\n"
              "\n"
              "void main() {\n"
              "float r = length(coord_);\n"
              "if(r > 1.0){ discard; }\n"
              "   color_out = color_ex;\n"
              "}\n";

      bool status = build_program_from_source(program_, "sphere", vert_source, frag_source);

      if(!status){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position_");
      mvpHandle_ = glGetUniformLocation(program_, "mvp_");
      colorHandle_ = glGetUniformLocation(program_, "solid_color_");
      offsetHandle_ = glGetUniformLocation(program_, "offset_");
      radiusHandle_ = glGetUniformLocation(program_, "radius_");

#else

      bool status = build_program(program_, "SpriteProgram",
                                  vert_shader.c_str(), frag_shader.c_str());
      if(!status){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position_in_");
      colorHandle_ = glGetUniformLocation(program_, "solid_color_");
      offsetHandle_ = glGetUniformLocation(program_, "offset_");
      radiusHandle_ = glGetUniformLocation(program_, "radius_");

      mvpHandle_ = glGetUniformLocation(program_, "mvp_");

      return check_GL_error("SpriteProgram::build() exit");

#endif
      return check_GL_error("SphereProgram::build() exit");
    }

  };


  /**
   *
   * Program for lots of individual sprites
   *
   */
  struct SpriteProgram{

    // Shaders
    std::string vert_shader = "./assets/shaders/sprites.vert";
    std::string frag_shader = "./assets/shaders/sprites.frag";

    // Program handle
    GLuint program_;

    // Attributes
    GLint positionHandle_;
    GLint colorHandle_;

    // Size of sprite in pixels
    GLint sizeHandle_;

    // Uniform handles
    GLint mvpHandle_;
    GLint offsetHandle_;

    /**
     * Initialize resources
     */
    bool init_resources(){
      program_ = glCreateProgram();

      return true;
    }

    void cleanup(){
      glDeleteProgram(program_);
    }

    bool build(){

      bool status = build_program(program_, "SpriteProgram",
                                  vert_shader.c_str(), frag_shader.c_str());
      if(!status){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position_in_");
      colorHandle_ = glGetAttribLocation(program_, "color_in_");
      sizeHandle_ = glGetAttribLocation(program_, "size_in_");

      mvpHandle_ = glGetUniformLocation(program_, "mvp_");
      offsetHandle_ = glGetUniformLocation(program_, "offset_");

      return check_GL_error("SpriteProgram::build() exit");
    }

  };

  /**
   * Manages a tiled set of images. Each image tile is the same size.
   *
   */
  struct ImageAtlas{

    int tex_unit_ = 0;

    int tile_width_ = 0;
    int tile_height_ = 0;

    // Texture Size
    int tex_width_ = 0;
    int tex_height_ = 0;

    // How many tiles in each dimension
    int tiles_x_ = 0;
    int tiles_y_ = 0;

    int capacity_ = 0;

    int count_ = 0;

    const static int max_texture_width_ = 1024;

    GLuint tex_;

    /**
     *
     * Tiles are packaged tight. You are responsible for putting borders
     *
     * @param tex_unit  Texture unit to use
     * @param width
     * @param height
     * @param layers
     * @return
     */
    ImageAtlas(int tex_unit, int tile_width, int tile_height, int capacity)
        : tex_unit_(tex_unit),
          tile_width_(tile_width), tile_height_(tile_height),
          capacity_(capacity){

      // Compute maximum number of tiles we can do in the x direction
      tiles_x_ = max_texture_width_ / float(tile_width);

      tiles_y_ = ceil(capacity / float(tiles_x_));


      // TODO: HACK!!!
      if(capacity == 1){
        tiles_x_ = 1;
      }

      tex_width_ = tile_width * tiles_x_;
      tex_height_ = tile_height * tiles_y_;

      std::cout<<"ImageAtlas::constructor:: "<<std::endl
               <<"ImageAtlas:: Unit : "<<tex_unit_<<std::endl
               <<"ImageAtlas:: Capacity : "<<capacity_<<"  tiles."<<std::endl
               <<"ImageAtlas:: Tiles : "<<tiles_x_<<" x "<<tiles_y_<<std::endl
               <<"ImageAtlas:: Size : "<<tile_width<<" x "<<tile_height<<std::endl
               <<"ImageAtlas:: Texture : "<<tex_width_<<" x "<<tex_height_<<std::endl;
    }

    ImageAtlas(int tex_unit, int tile_width, int tile_height, int tiles_x, int tiles_y)
        : tex_unit_(tex_unit),
          tile_width_(tile_width), tile_height_(tile_height),
          tex_width_(tiles_x * tile_width), tex_height_(tiles_y * tile_height),
          tiles_x_(tiles_x), tiles_y_(tiles_y),
          capacity_(tiles_x * tiles_y){



      std::cout<<"ImageAtlas::constructor:: "<<std::endl
               <<"ImageAtlas:: Unit : "<<tex_unit_<<std::endl
               <<"ImageAtlas:: Tiles : "<<tiles_x_<<" x "<<tiles_y_<<std::endl
               <<"ImageAtlas:: Capacity : "<<capacity_<<"  tiles."<<std::endl
               <<"ImageAtlas:: Size : "<<tile_width<<" x "<<tile_height<<std::endl
               <<"ImageAtlas:: Texture : "<<tex_width_<<" x "<<tex_height_<<std::endl;
    }

    /**
     * ImageAtlas::init_resources
     *
     * @return
     */
    virtual bool init_resources() {
      std::cout<<"ImageAtlas::init_resources()"<<std::endl;

      // We want to clear the texture and this is sort of a hackish way to do so.
      char *zero_buffer = new char[4 * tex_width_ * tex_height_];

      for(int i = 0; i < 4 * tex_width_ * tex_height_; ++i){
        zero_buffer[i] = 0;
      }

      // Create the textures we need
      glGenTextures(1, &tex_);

      glActiveTexture(GL_TEXTURE0 + tex_unit_);
      glBindTexture(GL_TEXTURE_2D, tex_);
      glPixelStorei(GL_PACK_ALIGNMENT, 4);

      // These have to be after the load or it doesn't work.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero_buffer);

//      std::cout<<"Creating texture with size "<<tex_width_<<" x "<<tex_height_<<" on unit "<<tex_unit_<<std::endl;

      delete[] zero_buffer;
      return true;
    }

    /**
     * ImageAtlas::bind
     *
     */
    void bind(){
      glBindTexture(GL_TEXTURE_2D, tex_);
    }

    void cleanup() {
      glDeleteTextures(1, &tex_);
    }

    /**
     * ImageAtlas::get_tile_coordinates
     *
     * Get the texture coordinates that reference the given tile
     *
     * @param index
     * @param tex_uv  (u_0, v_0, u_1, v_1),
     */
    void get_tile_coordinates(int index, float tex_uv[]){

      if(count_ <= 0){
        std::cerr<<"ImageAtlas::get_tile_coordinates: No tiles"<<std::endl;
        return;
      }

      // TODO: Factor out
      float tex_dx = 1.0f / float(tiles_x_);
      float tex_dy = 1.0f / float(tiles_y_);

      int tile_index = index % count_;
      int ix = tile_index % tiles_x_;
      int iy = tile_index / tiles_x_;

      tex_uv[0] = ix * tex_dx;
      tex_uv[1] = (iy + 1) * tex_dy;

      tex_uv[2] = (ix + 1) * tex_dx;
      tex_uv[3] = iy * tex_dy;
    }

    /**
     *
     * ImageAtlas::add_tile
     *
     * @return
     */
    int add_tile(){

      if(count_ >= capacity_){
        std::cerr<<"ImageAtlas::add_tile: Can't add new tile because atlas is at capacity "<<count_<<"/"<<capacity_<<std::endl;
        throw 1;
      }

      int index = count_;
      ++count_;

      return index;
    }

    /**
     * ImageAtlas::add_tile
     *
     * Add a new tile
     *
     * @param index
     * @param buffer
     * @return
     */
    int add_tile(const unsigned char *buffer){

      int index = add_tile();

 //     std::cout<<"ImageAtlas::add_tile "<<index<<std::endl;

      update_tile(index, buffer);

      return index;
    }

    /**
     * ImageAtlas::update_tile
     *
     *
     * @param index
     * @param buffer
     * @return
     */
    bool update_tile(int index, const unsigned char *buffer){

//      std::cout<<"ImageAtlas::update_tile "<<index<<std::endl;

      if(index >= count_){
        std::cerr<<"ImageAtlas::update_tile :  Index "<<index<<" out of range  "<<capacity_<<std::endl;
        return false;
      }

      int tile_x = index % tiles_x_;
      int tile_y = index / tiles_x_;

      return upload_tile(tile_x, tile_y, buffer);
    }


    /**
     *
     * @param col
     * @param row
     * @param width
     * @param height
     */
    bool upload_tile(int tile_x, int tile_y, const void *buffer){

      check_GL_error("ImageAtlas::upload_tile() entry");

  //    std::cout<<"ImageAtlas::Upload_tile "<<tile_x<<", "<<tile_y<<std::endl;

      if(!((tile_x < tiles_x_) && (tile_y < tiles_y_))){

        std::cerr<<" col or row is out of range "<<tile_x<<" of "<<tiles_x_
                 <<" "<<tile_y<<" of "<<tiles_y_<<std::endl;
        return false;
      }

      int x = tile_x * tile_width_;
      int y = tile_y * tile_height_;

      glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, tile_width_, tile_height_,
                      GL_RGBA, GL_UNSIGNED_BYTE, buffer);
      // TODO: Following two lines probably have a negative effect on perf

      // Need to have this here otherwise the caller can change buffer while we are editing it
      glFlush();
      glGenerateMipmap(GL_TEXTURE_2D);

//      std::cout<<"Uploading image of size "<<tile_width_<<", "<<tile_height_<<std::endl;

      return check_GL_error("ImageAtlas::upload_tile() exit");
    }

    /**
     *
     */
    void save_atlas(const char *filename) {

      using namespace std;

      ImageAtlas::save_atlas(tex_, tex_width_, tex_height_, filename);
    }

    static void save_atlas(GLuint tex, int width, int height, const char *filename) {
      QImage img(width, height, QImage::Format_RGBA8888);

      check_GL_error("ImageAtlas::save_atlas() entry");

      glBindTexture(GL_TEXTURE_2D, tex);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

      std::cout << "ImageAtlas::save_atlas - Writing image of size " << width << " by " << height
                << " filename " << filename << std::endl;

      img.save(filename);
    }

  };

  /**
   * Draw a full screen image
   */
  struct FullScreenImage : public FlatShape{

    /**
     * Settings
     */

    // Shaders
    std::string vert_shader = "./assets/shaders/identity.vert";
    std::string frag_shader = "./assets/shaders/background.frag";

    /**
     * Data
     */
    // Program handle
    GLuint program_ = 0;

    // Position Attribute
    GLint positionHandle_ = 0;

    GLint backgroundHandle_ = 0;
    GLint backgroundAlpha_ = 0;
    GLint backgroundTime_ = 0;
    GLint backgroundScale_ = 0;

    // Background texture
    GLuint tex_ = 0;


    /**
     * Texture
     */
    int tex_unit_ = 0;

    int tex_width_ = 0;
    int tex_height_ = 0;
    int layers_ = 0;

    float alpha_ = 0.0f;
    float time_ = 0.0f;

    // Scale of quad
    float scale_[3] = {1.0f, 1.0f, 1.0f};

    FullScreenImage(int texture_unit, int tex_width, int tex_height, int tex_depth)
        : tex_unit_(texture_unit), tex_width_(tex_width), tex_height_(tex_height),
          layers_(tex_depth){

    }

    /**
     *
     * @return
     */
    bool init_resources(){

      std::cout<<"FullScreenImage::init_resources"<<std::endl;

      /**
       * Build programs
       */
      program_ = glCreateProgram();

      bool status = build_program(program_, "FullScreenImage",
                                  vert_shader.c_str(), frag_shader.c_str());
      if(!status){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position_in_");

      backgroundHandle_ = glGetUniformLocation(program_, "background_");
      backgroundAlpha_ = glGetUniformLocation(program_, "background_alpha_");
      backgroundTime_ = glGetUniformLocation(program_, "background_time_");
      backgroundScale_ = glGetUniformLocation(program_, "scale_");

      /**
       * Initialize textures
       */
      // Create the textures we need
      glGenTextures(1, &tex_);

      glActiveTexture(GL_TEXTURE0 + tex_unit_);
      glBindTexture(GL_TEXTURE_3D, tex_);
      glPixelStorei(GL_PACK_ALIGNMENT, 1);

      // These have to be after the load or it doesn't work.
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, tex_width_, tex_height_, layers_,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      /**
       * Create geometry
       */
      FlatShape::init_resources();
      FlatShape::setup_array(positionHandle_);
      FlatShape::build_triangle(2.0);

//      enable(true);

      return check_GL_error("FullScreenImage::init_resources() exit");
    }

    /**
     * FullScreenImage::cleanup
     *
     *  Free OpenGL resources
     *
     */
    void cleanup(){
      glDeleteProgram(program_);
      glDeleteTextures(1, &tex_);
    }

    void setScaleFromAspect(float aspect){
      scale_[0] = 1.0f;
      scale_[1] = aspect;
    }

    /**
     * FullScreenImage::load_image
     *
     * Load a slice into the 3d texture
     *
     */
    void upload_slice(int layer, const void *buffer){

      check_GL_error("FullScreenImage::upload_slice() entry");

      if(layer > layers_){
        std::cerr<<" FullScreenImage::upload_slice(): layer "<<layer<<" out of range "<<std::endl;
        return;
      }
#if 0
      std::cout<<"FullScreenImage::upload_slice(): Uploading image of size "<<tex_width_
               <<", "<<tex_height_<<" on layer "<<layer+1<<" of "<<layers_<<std::endl;
#endif
      glBindTexture(GL_TEXTURE_3D, tex_);
      glTexSubImage3D(GL_TEXTURE_3D, 0,
                      0, 0, layer,
                      tex_width_, tex_height_, 1,
                      GL_RGBA, GL_UNSIGNED_BYTE, buffer);

      glGenerateMipmap(GL_TEXTURE_3D);

      // Need to have this here otherwise the caller can change buffer while we are editing it
      glFlush();

      check_GL_error("FullScreenImage::upload_slice() exit");
    }

    /**
     * FullScreenImage::render
     *
     * @param camera
     */
    virtual void render(Camera<float>& camera){
      // Draw
      glUseProgram(program_);

      glUniform1i(backgroundHandle_, tex_unit_);
      glUniform1f(backgroundAlpha_, alpha_);
      glUniform1f(backgroundTime_, time_);
      glUniform3fv(backgroundScale_, 1, scale_);

      FlatShape::bind();
      FlatShape::render(camera);

      check_GL_error("FullScreenImage::render()");
    }
  };

  /**
   *
   * Responsible for displaying a set of textured quads (TODO: or triangles?)
   * where each quad's texture is a tile from a texture atlas.
   *
   * Each quad is represented by 2 triangles and the quad has it's center at the origin.
   * This can be used for drawing a whole bunch of galaxies or
   * texture labels.

   *
   * There are essentially three potential ways for specifying the data for the quads.
   *
   *  1. Vertex and texture data completely and make any changes on the CPU
   *  2. Fixed vertex and texture data + custom MV matrix for each billboard
   *  3. Fixed vertex and texture data + individual components like rotation, scale, offset and have GLSL create matrices
   *
   *  Approach 1 would almost be simplest and will perform just fine in most cases.
   *
   *  Approach 2 seems sort of the best compromise but it's weird and tricky to submit matrices as attribute.
   *
   *  Approach 3 seems computationally wasteful but it should be a bit easier to code up.
   *
   *
   * Doing Approach 3
   *
   * Inside the VBO the fixed data (vertex coordinates and texture coordinates) is interleaved as and the auxilary
   * attribute array is appended at the end. This makes changing it easier. Example:
   *
   *   v_0, v_1, t_0, t_1, ... v_n, ; dx_0, dx_0, dtheta_0, ds_0, dt_1
   *
   */

  struct BillboardSet : public msg::Node {

    // Quad dimensions
    float quad_width_ = 0.1f;
    float quad_height_ = 0.1f;

    // Global transparancy of the billboards
    float global_opacity_ = 1.0f;

    // Maximum number of billboards
    int capacity_ = 0;

    // Number of vertices for each billboard
    const static int verts_per_board_ = 6;

    // Element Stride for the static interleaved array (3 position, 2 texture, 4 color)
    const static int attributes_per_vert_ =  9;

    // Attribute indices
    const static int ix_position_ = 0;
    const static int ix_texture_ = 3;
    const static int ix_color_ = 5;

    // Number of billboards in array
    int count_ = 0;

    int atlas_tex_unit_ = 0;

    // TODO: This can be uploaded directly if we are a bit clever with offsets
    struct s_billboard{
      float position[3];
      float rotation;
      float scale[2];
      float tex[4];
      float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};

      bool enabled;

    } *info_;

    // TODO: Would be better to use above struct directly since this is only used for upload
    float *vertex_data_ = nullptr;

    int data_size_ = 0;

    // Shaders
    std::string vert_shader = "./assets/shaders/billboard_set.vert";
    std::string frag_shader = "./assets/shaders/billboard_set.frag";

    // Program handle
    GLint program_;

    // Attributes
    GLint positionHandle_;
    GLint textureHandle_;
    GLint colorHandle_;

    // Uniform handles
    GLint mvpHandle_;
    GLint samplerHandle_;
    GLint opacityHandle_;

    GLuint vao = 0;
    GLuint vbo = 0;

    BillboardSet(int max_billboards)
        : capacity_(max_billboards){

      info_ = new s_billboard[max_billboards];

      for(int i = 0; i < max_billboards; ++i){
        info_[i].enabled = false;
        info_[i].position[0] = 0.0f;
        info_[i].position[1] = 0.0f;
        info_[i].position[2] = 0.0f;
        info_[i].rotation = 0.0f;
        info_[i].scale[0] = 1.0f;
        info_[i].scale[1] = 1.0f;
      }

      data_size_ = attributes_per_vert_ * verts_per_board_ * capacity_ ;

      vertex_data_ = new float[data_size_];

      for(int i = 0; i < data_size_; ++i){
        vertex_data_[i] = 0.0f;
      }

    }

    ~BillboardSet(){
      delete[] info_;
      delete[] vertex_data_;
    }


    /**
     * BillboardSet init_resources
     *
     * @return
     */
    bool init_resources(){
//      std::cout<<"BillboardSet::init_resources()"<<std::endl;

      check_GL_error("BillboardSet::init_resources() entry");

      program_ = glCreateProgram();

      // Build programs
      if(!build_program(program_, "BillboardSet", vert_shader.c_str(), frag_shader.c_str())){
        return false;
      }
      check_GL_error("BillboardSet::init_resources() 1");

 //     std::cout<<"program_ "<<program_<<std::endl;

      // Static properties
      positionHandle_ = glGetAttribLocation(program_, "position_in_");
      textureHandle_ = glGetAttribLocation(program_, "tex_in_");
      colorHandle_ = glGetAttribLocation(program_, "color_in_");

      // TODO: Can we get rid of this ??
      mvpHandle_ = glGetUniformLocation(program_, "mvp_");
      samplerHandle_ = glGetUniformLocation(program_, "atlas_");
      opacityHandle_ = glGetUniformLocation(program_, "global_alpha_");

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, data_size_ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

      /**
       * Static Data
       */
      int stride = sizeof(float) * attributes_per_vert_;

      glVertexAttribPointer(positionHandle_, 3, GL_FLOAT, GL_FALSE, stride, (void *) (ix_position_ * sizeof(float)));
      glEnableVertexAttribArray(positionHandle_);

      if(textureHandle_ > 0){
        glVertexAttribPointer(textureHandle_, 2, GL_FLOAT, GL_FALSE, stride, (void *) (ix_texture_ * sizeof(float)));
        glEnableVertexAttribArray(textureHandle_);
      }

      if(colorHandle_ > 0){
        glVertexAttribPointer(colorHandle_, 4, GL_FLOAT, GL_FALSE, stride, (void *) (ix_color_ * sizeof(float)));
        glEnableVertexAttribArray(colorHandle_);
      }


      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      return check_GL_error("BillboardSet::init_resources() exit");
    }


    void cleanup(){
      glDeleteProgram(program_);

      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
    }

    /**
     * Arrange all the billboards in a grid
     */
    void init_grid(int tiles_x){
      // Random
      float random_scale = 0.0f;

      // Initialize Random Number Generator
      std::random_device rd;
      std::mt19937 generator(rd());
      std::uniform_real_distribution<> random(-random_scale, random_scale);

//      const int default_tiles_x = 10;

      int tiles_y = count_ / tiles_x;

      // Initialize a grid
      float dx = 2.0f / float(tiles_x);
      float dy = 2.0f / float(tiles_y);
      float dz = -0.0001f;

      float dx_gap = 1.0f * dx;
      float dy_gap = 0.0f;

      for(int i = 0; i < count_; ++i){

        int tile_ix = i % tiles_x;
        int tile_iy = i / tiles_x;

        float x = -2.0f + (dx+dx_gap) * tile_ix;
        float y = -1.0f + (dy+dy_gap) * tile_iy;

        info_[i].position[0] = x;// + random_scale * random(generator);
        info_[i].position[1] = y;// + random_scale * random(generator);
        info_[i].position[2] = dz * i;
      }

      // Generate and upload the billboards
      upload_billboards();
    }

    /**
     *
     * Repack and upload the data. Texture coordinates and offsets are taken from the
     * .info array of structs. Offsets are used to compute the vertices for the quads.
     *
     * For custom billboards populate the .info[] array of structs and then call this
     * method.
     *
     */
    void upload_billboards(){

      using namespace std;

      check_GL_error("BillboardSet::upload_billboards() entry");
#if 0
      cout << "BillboardSet::upload_billboards "<< count_ << " with capacity "<<capacity_<<endl;
#endif
      // Vertices for the galaxy billboards
      // TODO: HACK
      //float data[attributes_per_vert_ * verts_per_board_ * 1024];

      int offset = 0;

      for(int i = 0; i < count_; ++i){

        // Data that is the same for all vertices in the bill-board (color, ?) TODO: Remove this I think.
        const float data_quad[4] = {info_[i].color[0], info_[i].color[1], info_[i].color[2], info_[i].color[3]};

        // Texture coordinates
        float tex_u_0 = info_[i].tex[0];
        float tex_v_0 = info_[i].tex[1];
        float tex_u_1 = info_[i].tex[2];
        float tex_v_1 = info_[i].tex[3];

#if 0
        // Positions for the 4 corners of the quad
        const float offset_x = info_[i].position[0];
        const float x_right = 0.5f * quad_width_ * info_[i].scale[0] + offset_x;
        const float x_left = -0.5f * quad_width_ * info_[i].scale[0] + offset_x;

        const float offset_y = info_[i].position[1];
        const float y_top =  0.5f * quad_height_ * info_[i].scale[1] + offset_y;
        const float y_bottom = -0.5f * quad_height_ * info_[i].scale[1] + offset_y;

        // Vertex and Texture data for the quad
        const float data_vert[verts_per_board_][5]{{x_left,  y_top,  0.0f, tex_u_0, tex_v_1},
                                                   {x_right, y_top,  0.0f, tex_u_1, tex_v_1},
                                                   {x_left,  y_bottom, 0.0f, tex_u_0, tex_v_0},
                                                   {x_right, y_top,  0.0f, tex_u_1, tex_v_1},
                                                   {x_left,  y_bottom, 0.0f, tex_u_0, tex_v_0},
                                                   {x_right, y_bottom, 0.0f, tex_u_1, tex_v_0}};
#else
        // Rotation "matrix"
        float sin_t = sin(info_[i].rotation);
        float cos_t = cos(info_[i].rotation);

        // Positions for the 4 corners of the quad
        const float offset_x = info_[i].position[0];
        const float x_right = 0.5f * quad_width_ * info_[i].scale[0];
        const float x_left = -0.5f * quad_width_ * info_[i].scale[0];

        const float offset_y = info_[i].position[1];
        const float y_top =  0.5f * quad_height_ * info_[i].scale[1];
        const float y_bottom = -0.5f * quad_height_ * info_[i].scale[1];

        // Rotate Quad
        const float x_rt = cos_t * x_right - sin_t * y_top + offset_x;
        const float y_rt = sin_t * x_right + cos_t * y_top + offset_y;

        const float x_rb = cos_t * x_right - sin_t * y_bottom + offset_x;
        const float y_rb = sin_t * x_right + cos_t * y_bottom + offset_y;

        const float x_lt = cos_t * x_left - sin_t * y_top + offset_x;
        const float y_lt = sin_t * x_left + cos_t * y_top + offset_y;

        const float x_lb = cos_t * x_left - sin_t * y_bottom + offset_x;
        const float y_lb = sin_t * x_left + cos_t * y_bottom + offset_y;

        // Vertex and Texture data for the quad
        const float data_vert[verts_per_board_][5]{{x_lt,  y_lt,  0.0f, tex_u_0, tex_v_1},
                                                   {x_rt, y_rt,  0.0f, tex_u_1, tex_v_1},
                                                   {x_lb,  y_lb, 0.0f, tex_u_0, tex_v_0},
                                                   {x_rt, y_rt,  0.0f, tex_u_1, tex_v_1},
                                                   {x_lb,  y_lb, 0.0f, tex_u_0, tex_v_0},
                                                   {x_rb, y_rb, 0.0f, tex_u_1, tex_v_0}};
#endif

#if 0
        cout << " Adding quad " << setw(4) << ix << " x " << setw(4) << iy << " at" << endl;
          for(int j = 0; j < verts_per_board_; j++){
              cout<<"Vertex "<<setw(2)<< j;

            // Per-vertex data
            for(int i = 0; i < 5; i++){
              cout<<setw(6)<<data_vert[j][i];
            }

            // Per-vertex data
            for(int i = 0; i < 3; i++){
              cout<<setw(6)<<data_quad[i];
            }
            cout<<endl;
          }
#endif

        // Load the triangle data
        for(int j = 0; j < verts_per_board_; j++){

          // Per-vertex data
          for(int i = 0; i < 5; i++){
            vertex_data_[offset++] = data_vert[j][i];
          }

          // Per-quad data
          for(int i = 0; i < 4; i++){
            vertex_data_[offset++] = data_quad[i];
          }
        }
      }

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, attributes_per_vert_ * verts_per_board_ * capacity_* sizeof(float), vertex_data_);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      check_GL_error("BillboardSet::upload_data() exit");
    }

    /**
     * Render BillboardSet
     *
     * @param camera_
     */
    virtual void render(Camera<float>& camera_){
      check_GL_error("BillboardSet::render() enter");
#if 0
      std::cout<<"BillboardSet::render: "<<count_
               <<" billboards using texture unit "<<atlas_tex_unit_
               <<" samplerHandle_ "<<samplerHandle_<<std::endl;
#endif
      // Draw
      glUseProgram(program_);
      glBindVertexArray(vao);

      glUniformMatrix4fv(mvpHandle_, 1, GL_FALSE, camera_.mvp);
      glUniform1i(samplerHandle_, atlas_tex_unit_);
      glUniform1f(opacityHandle_, global_opacity_);

      glDrawArrays(GL_TRIANGLES, 0,  6 * count_);
//      glDrawArrays(GL_POINTS, 0,  6 * count_);
      glBindVertexArray(0);
      check_GL_error("BillboardSet::render() exit");

    }

    /**
     *
     * @param world_x
     * @param world_y
     * @return
     */
    virtual bool hitTest(float world_x, float world_y){
      using namespace std;

      int hit_ix = -1;

      hit_ix = hitSelect(world_x, world_y);

//      std::cout<<"BillboardSet::hitTest Hit Index "<<hit_ix<<std::endl;

      return hit_ix >= 0;
    };


    int hitSelect(float x, float y){

      using namespace std;

      int hit_ix = -1;

      float hit_radius_sq = quad_width_ * quad_width_
                            + quad_height_ * quad_height_;

      float r_sq_min = 2 * hit_radius_sq;

      for(int i = 0; i < count_; ++i){

        float dx = (info_[i].position[0] - x);
        float dy = (info_[i].position[1] - y);

        float r_sq = dx*dx + dy*dy;

        if(r_sq < r_sq_min){
          r_sq_min = r_sq;
          hit_ix = i;
        }
      }

      return hit_ix;
    }
  };


  /**
   *
   * Contains program, texture and geometry
   *
   */
  struct TexturedSphere : public Billboard{

    // Shaders
    std::string vert_shader = "./assets/shaders/sphere.vert";
    std::string frag_shader = "./assets/shaders/textured_sphere.frag";

    // Settings
    float obliquity = 0.0f;
    float inclination_ = 0.0f;

    // Internal

    // Texture Size
    int tex_width_ = 0;
    int tex_height_ = 0;

    int capacity_ = 0;

    int count_ = 0;

    const static int max_texture_width_ = 1024;

    /**
     *  OpenGL
     */

    // Texture handles
    GLuint tex_;
    GLint tex_unit_ = 0;

    // Program handle
    GLuint program_;

    // Position Attribute
    GLuint positionHandle_;

    // Uniform handles
    GLint mvpHandle_;

    GLint solidColorHandle_;
    GLint offsetHandle_;
    GLint radiusHandle_;


    // camera up world
    GLint cameraUpHandle_;
    GLint cameraEyeHandle_;
    GLint cameraRightHandle_;


    GLint colorSamplerHandle_;

    virtual bool init_resources() {
//      std::cout<<"TexturedSphere::init_resources()"<<std::endl;
      check_GL_error("TexturedSphere::init_resources() entry");

      Billboard::init_resources();

      /**
       * Build programs
       */
      program_ = glCreateProgram();

      if(!build_program(program_, "TextureSphereProgram",
                        vert_shader.c_str(), frag_shader.c_str())){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position_in_");

      mvpHandle_ = glGetUniformLocation(program_, "mvp_");

      solidColorHandle_ = glGetUniformLocation(program_, "solid_color_");
      radiusHandle_ = glGetUniformLocation(program_, "radius_");
      offsetHandle_ = glGetUniformLocation(program_, "offset_");

      colorSamplerHandle_ = glGetUniformLocation(program_, "color_sampler_");

      cameraUpHandle_ = glGetUniformLocation(program_, "camera_up_world_");
      cameraEyeHandle_ = glGetUniformLocation(program_, "camera_eye_world_");
      cameraRightHandle_ = glGetUniformLocation(program_, "camera_right_world_");

      setup_array(positionHandle_);

      /**
       * Initialize Textures
       */
      glGenTextures(1, &tex_);

      glActiveTexture(GL_TEXTURE0 + tex_unit_);
      glBindTexture(GL_TEXTURE_2D, tex_);
//      glPixelStorei(GL_PACK_ALIGNMENT, 4);

      // These have to be after the load or it doesn't work.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      return check_GL_error("TexturedSphere::init_resources() exit");
    }

    void cleanup() {
      glDeleteTextures(1, &tex_);
      glDeleteProgram(program_);

      Billboard::cleanup();
    }

    // Load the data
    bool init_from_file(int tex_unit, const char* filename){

      using namespace std;

      cout<<"TexturedSphere::init_from_file "<<filename<<" on unit "<<tex_unit<<endl;
      check_GL_error("TexturedSphere::init_from_file() - entry");

      QImageReader reader(filename);
      const QImage source_img = reader.read();

      tex_width_ = source_img.width();
      tex_height_ = source_img.height();
      tex_unit_ = tex_unit;

      // Need an RGBA image so have to draw it ourselves.
      QImage img(tex_width_, tex_height_, QImage::Format_RGBA8888);
      QPainter painter(&img);
      painter.drawImage(source_img.rect(), source_img);

      TexturedSphere::init_resources();

      glBindTexture(GL_TEXTURE_2D, tex_);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

      glGenerateMipmap(GL_TEXTURE_2D);

      // Need to have this here otherwise the caller can change buffer while we are editing it
      glFlush();

      check_GL_error("TexturedSphere::init_from_file - exit");
      return true;
    }

    virtual void render(Camera<float>& camera){
//      std::cout<<"TexturedSphere::render() "<<radius<<std::endl;

      glUseProgram(program_);
      glUniformMatrix4fv(mvpHandle_, 1, GL_FALSE, camera.mvp);

      glUniform3fv(cameraEyeHandle_, 1, camera.eye_world);
      glUniform3fv(cameraUpHandle_, 1, camera.up_world);
      glUniform3fv(cameraRightHandle_, 1, camera.right_world);

      glUniform1i(colorSamplerHandle_, tex_unit_);
      glUniform1f(radiusHandle_, radius);
      glUniform3fv(offsetHandle_, 1, position);

      Billboard::render(camera);

    }

  };


  /**
   * Used to draw clouds, etc.
   */
  struct ProceduralSphere : public Billboard{

    /**
     * Settings
     */

    // Shaders
    std::string vert_shader = "./assets/shaders/sphere.vert";
    std::string frag_shader = "./assets/shaders/sphere.frag";

    float color_[4] = {0.9f, 0.9f, 1.0f, 1.0f};

    /**
     * Data
     */
    int capacity_ = 0;
    int count_ = 0;


    /**
     *  OpenGL
     */

    // Program handle
    GLuint program_;

    // Position Attribute
    GLuint positionHandle_;

    // Uniform handles
    GLint mvpHandle_;

    // World view
    GLint cameraUpHandle_;
    GLint cameraEyeHandle_;
    GLint cameraRightHandle_;

    GLint colorHandle_;
    GLint offsetHandle_;
    GLint radiusHandle_;

    GLint colorSamplerHandle_;


    virtual bool init_resources() {
      check_GL_error("ProceduralSphere::init_resources() entry");

      Billboard::init_resources();

      /**
       * Build programs
       */
      program_ = glCreateProgram();

      if(!build_program(program_, "ProceduralSphereProgram",
                        vert_shader.c_str(), frag_shader.c_str())){
        return false;
      }

      positionHandle_ = glGetAttribLocation(program_, "position_in_");

      mvpHandle_ = glGetUniformLocation(program_, "mvp_");

      cameraUpHandle_ = glGetUniformLocation(program_, "camera_up_world_");
      cameraEyeHandle_ = glGetUniformLocation(program_, "camera_eye_world_");
      cameraRightHandle_ = glGetUniformLocation(program_, "camera_right_world_");

      colorHandle_ = glGetUniformLocation(program_, "C_base");
      radiusHandle_ = glGetUniformLocation(program_, "radius_");
      offsetHandle_ = glGetUniformLocation(program_, "offset_");

      setup_array(positionHandle_);

      return check_GL_error("TexturedSphere::init_resources() exit");
    }

    void cleanup() {
      glDeleteProgram(program_);

      Billboard::cleanup();
    }

    virtual void render(Camera<float>& camera){

#if 0
      std::cout<<"ProceduralSphere::render() "<<radius<<std::endl;
      check_GL_error("ProceduralSphere::render() enter");
#endif
      glUseProgram(program_);
      glUniformMatrix4fv(mvpHandle_, 1, GL_FALSE, camera.mvp);

      glUniform3fv(cameraEyeHandle_, 1, camera.eye_world);
      glUniform3fv(cameraUpHandle_, 1, camera.up_world);
      glUniform3fv(cameraRightHandle_, 1, camera.right_world);

      glUniform1f(radiusHandle_, radius);
      glUniform3fv(offsetHandle_, 1, position);
      glUniform4fv(colorHandle_, 1, color_);

      check_GL_error("ProceduralSphere::render() exit");

      Billboard::render(camera);
    }

  };

// end of namespace msg;
}

#endif //ASTROLABS_SCENE_GRAPH_H
