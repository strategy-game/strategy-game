#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <stddef.h>
#include <GL/gl.h>

class Texture {
public:
	Texture() {};
	Texture(const char * path);
	~Texture();
	
	uint32_t * buffer;
	size_t width;
	size_t height;
	GLuint gl_tex_num;

	void from_file(const char * path);
	void to_opengl();
	void delete_opengl();
};

#endif