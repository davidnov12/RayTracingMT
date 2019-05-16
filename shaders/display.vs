/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* display.vs
*/

#version 450

out vec2 uv;	// Output texture coordinations

void main(){
	
	// Setup fullscreen quad (positions + texcoords)
	if(gl_VertexID == 0){
		gl_Position = vec4(-1,-1,0,1);
		uv = vec2(0, 0);
	}
	
	if(gl_VertexID == 1){
		gl_Position = vec4(-1,1,0,1);
		uv = vec2(0, 1);
	}
	
	if(gl_VertexID == 2){
		gl_Position = vec4(1,-1,0,1);
		uv = vec2(1, 0);
	}
	
	if(gl_VertexID == 3){ 
		gl_Position = vec4(1,1,0,1);
		uv = vec2(1, 1);
	}
}