/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* display.fs
*/

#version 450


uniform sampler2D screen;	// Ray traced image

in vec2 uv;					// Texture coordinations of this fragment

out vec4 color;				// Output color

void main(){
	
	// Sample texture
	color = texture(screen, uv);
		
}