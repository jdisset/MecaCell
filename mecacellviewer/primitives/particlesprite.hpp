#ifndef PARTICLE
#define PARTICLE
#include <vector>
#include <iostream>
#include <cmath>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLFunctions_3_3_Core>

class Particlesprite{
	protected:
		QOpenGLShaderProgram& shader;
		QOpenGLBuffer vbuf, pbuf, cbuf;
		QOpenGLVertexArrayObject* vao;
		std::vector<float> vertices  = {-1,-1,0,1,-1,0,-1,1,0,1,1,0};
		std::vector<float> positions = {};
		std::vector<float> colors = {};

	public:

		Particlesprite (QOpenGLShaderProgram& s):
			shader(s){}

		void load(){
			shader.bind();
			vao = new QOpenGLVertexArrayObject();
			vao->create();
			vao->bind();

			vbuf.create();
			vbuf.bind();
			vbuf.allocate( &vertices[0], vertices.size()*sizeof(float));
			shader.enableAttributeArray(0);
			shader.setAttributeBuffer(0, GL_FLOAT, 0, 3 );

			pbuf.create();
			pbuf.bind();
			pbuf.allocate( &positions[0], positions.size()*sizeof(float));
			pbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
			shader.enableAttributeArray(1);
			shader.setAttributeBuffer(1, GL_FLOAT, 0, 3 );

			cbuf.create();
			cbuf.bind();
			cbuf.allocate( &colors[0], colors.size()*sizeof(float));
			cbuf.setUsagePattern(QOpenGLBuffer::StreamDraw);
			shader.enableAttributeArray(2);
			shader.setAttributeBuffer(2, GL_FLOAT, 0, 4 );

			vao->release();
			shader.release();
		}

		void update (vector<pair<QVector3D,QVector4D>>& p){
			positions.clear();
			colors.clear();
			for(size_t i = 0 ; i < p.size() ; ++i){
				positions.push_back(p[i].first.x());
				positions.push_back(p[i].first.y());
				positions.push_back(p[i].first.z());
				colors.push_back(p[i].second.x());
				colors.push_back(p[i].second.y());
				colors.push_back(p[i].second.z());
				colors.push_back(p[i].second.w());
			}
			shader.bind();
			vao->bind();
			pbuf.bind();
			pbuf.allocate(&positions[0], positions.size()*sizeof(float));
			cbuf.bind();
			cbuf.allocate(&colors[0], colors.size()*sizeof(float));
			vao->release();
			shader.release();
		}

		void draw(QVector2D size,QVector3D& cameraUp, QVector3D& cameraRight, QMatrix4x4& VP){
			 QOpenGLFunctions_3_3_Core *GLf = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
			 GLf->initializeOpenGLFunctions();
			shader.bind();
			vao->bind();
			shader.setUniformValue(shader.uniformLocation("cameraUp"),cameraUp);
			shader.setUniformValue(shader.uniformLocation("cameraRight"),cameraRight);
			shader.setUniformValue(shader.uniformLocation("size"),size);
			shader.setUniformValue(shader.uniformLocation("VP"),VP);
			GLf->glVertexAttribDivisor(0, 0);
			GLf->glVertexAttribDivisor(1, 1);
			GLf->glVertexAttribDivisor(2, 1);
			GLf->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, positions.size()/3);
			vao->release();
			shader.release();

		}

		void draw(QVector2D size, GLuint t, QVector3D& cameraUp, QVector3D& cameraRight, QMatrix4x4& VP){
			shader.bind();
			vao->bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, t);
			shader.setUniformValue(shader.uniformLocation("tex"),0);
			shader.setUniformValue(shader.uniformLocation("cameraUp"),cameraUp);
			shader.setUniformValue(shader.uniformLocation("cameraRight"),cameraRight);
			shader.setUniformValue(shader.uniformLocation("size"),size);
			shader.setUniformValue(shader.uniformLocation("VP"),VP);
			glVertexAttribDivisor(0, 0);
			glVertexAttribDivisor(1, 1);
			glVertexAttribDivisor(2, 1);
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, positions.size()/3);
			vao->release();
			shader.release();

		}

		~Particlesprite(){
			delete vao;
		}
};


#endif

