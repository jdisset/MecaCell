#ifndef LINES
#define LINES
#include <vector>
#include <iostream>
#include <cmath>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>

class Lines{
   protected:
      QOpenGLShaderProgram& shader;
      QOpenGLBuffer vbuf;
      QOpenGLVertexArrayObject* vao;
      std::vector<float> vertices;

   public:

      Lines (QOpenGLShaderProgram& s):
         shader(s){}

      void load(){
         shader.bind();
         vao = new QOpenGLVertexArrayObject();
         vao->create();
         vao->bind();

         vbuf.create();
         vbuf.bind();
         vbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
         vbuf.allocate( &vertices[0], vertices.size()*sizeof(float));
         shader.enableAttributeArray(0);
         shader.setAttributeBuffer(0, GL_FLOAT, 0, 3 );

         vao->release();
         shader.release();
      }

      void updateVertices(std::vector<std::pair<QVector3D,QVector3D>>& v){
         vertices.clear();
         vertices.resize(0);
         for (unsigned int i = 0 ; i < v.size() ; i++){
            vertices.push_back(v[i].first.x());
            vertices.push_back(v[i].first.y());
            vertices.push_back(v[i].first.z());
            vertices.push_back(v[i].second.x());
            vertices.push_back(v[i].second.y());
            vertices.push_back(v[i].second.z());
         }
         shader.bind();
         vao->bind();
         vbuf.bind();
         vbuf.allocate(&vertices[0], vertices.size()*sizeof(float));
         vao->release();
         shader.release();
      }

      void draw( QMatrix4x4 &view, QMatrix4x4 &projection, QVector4D color, float w){
			auto GLF = QOpenGLContext::currentContext()->functions();
			GLF->glEnable(GL_LINE_SMOOTH);
			GLF->glLineWidth(w);
         shader.bind();
         vao->bind();
         shader.setUniformValue(shader.uniformLocation("projection"),projection);
         shader.setUniformValue(shader.uniformLocation("view"),view);
         shader.setUniformValue(shader.uniformLocation("color"),color);
         GLF->glDrawArrays(GL_LINES,0,vertices.size()/3.0);
         vao->release();
         shader.release();
      }

      ~Lines(){
         delete vao;
      }
};


#endif

