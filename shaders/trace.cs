/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* trace.cs
*/

#version 450

#extension GL_ARB_bindless_texture : require

#define MAX_DEPTH 3
#define EPS 0.0001
#define PI 3.14159

// Rendering modes
#define RAY_TRACING 0
#define HEATMAP 1

// Structure of material
struct Material{
  vec3 diffuseCol;
  float refIndex;
  sampler2D diffuseTex;
  float roughness;
  float metalness;
};

// Triangle structure
struct Triangle{

  // Vertice positions
	vec3 pos_a;
	vec3 pos_b;
	vec3 pos_c;

  // Normal vectors
	vec3 nor_a;
	vec3 nor_b;
	vec3 nor_c;

  // Texture coordinates
	vec2 uv_a;
	vec2 uv_b;
	vec2 uv_c;

  // material's ID
	int mat_id;
};

// Structure of ray
struct Ray{
	vec3 origin;
	vec3 direction;
	float energy;
};

// Point of collision between ray and triangle
struct CollisionPoint{
	float dist;
	vec3 position;
	vec3 color;
	vec3 normal;
  vec2 uvs;
  float refractionIndex;
  float metalness;
  float roughness;
};

// BVH node structure
struct Node{

  // Bounding volume coordinates
  vec4 min;
  vec4 max;

  // Indices to node's childs
  int left;
  int right;

  // Indices to node's primitives
  int first;
  int last;

  // Index of parent node
  int parent;

  // Additional info + memory alignment
  int gapa, gapb, gapc;
};

// Workgroup size
layout(local_size_x = 8, local_size_y = 8) in;

// Renderbuffer
layout(rgba32f, binding = 0) uniform image2D img_output;

// Triangles
layout (std430, binding = 1) buffer in_data {
	Triangle data[];
};

// Materials
layout (std430, binding = 2) buffer in_mats {
	Material materials[];
};

// Nodes of BVH
layout (std430, binding = 3) buffer in_nodes {
	Node tree[];
};

// Indices of triangles
layout (std430, binding = 4) buffer in_index {
	int indices[];
};

layout (std430, binding = 7) buffer idbg_index {
	float dbg[];
};


uniform vec3 screen_plane[3];
uniform vec3 view_pos;
uniform vec3 light_pos = vec3(4, 7, 1);

uniform int width;
uniform int height;

uniform int renderMode;
uniform int bvhType;

uniform int shadowSamples = 1;
uniform int indirectSamples = 1;
uniform int dofSamples = 1;
uniform int aoSamples = 1;

float heat = 0.0f;
//int traverseCount = 0;
//int dbg_cnt = 0;

// Functions prototypes
bool findCollision(Ray r, out CollisionPoint c);
bool bvhTraversal(Ray r, out CollisionPoint c);

/*
* Simple random number generator
*/
float randm(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

/* Barycentric interpolation
*
* tr - input triangle
* point - point for interpolation compute
* bar_coords - variable for result barycentric coordinates
*/
void barycentricInterpolation(Triangle tr, vec3 point, out vec3 bar_coords){

  vec3 v0 = tr.pos_b - tr.pos_a;
	vec3 v1 = tr.pos_c - tr.pos_a;
	vec3 v2 = point - tr.pos_a;

	float d00 = dot(v0, v0);
	float d01 = dot(v0, v1);
	float d11 = dot(v1, v1);
	float d20 = dot(v2, v0);
	float d21 = dot(v2, v1);

	float den = d00 * d11 - d01 * d01;

  bar_coords.y = (d11 * d20 - d01 * d21) / den;
	bar_coords.z = (d00 * d21 - d01 * d20) / den;
	bar_coords.x = 1.0 - bar_coords.y - bar_coords.z;
}

/* Computation of lighting in some point
*
* col - input triangle
* fresnell - point for interpolation compute
*/
vec3 computeLighting(CollisionPoint col, float fresnell){

  vec3 light_dir = normalize(light_pos - col.position);
  vec3 view_dir = normalize(view_pos - col.position);
  vec3 reflect_dir = reflect(-light_dir, col.normal);

  float amb = 0.3;
  float diff = 0.1, spec = 0.0;

  float increase = 1.0 / shadowSamples;
  vec3 indirect = vec3(0);

  Ray s;
  s.origin = col.position + (light_dir * 0.001);
  s.direction = light_dir;

  vec3 focusPoint = s.origin + s.direction;
  CollisionPoint v;

  if(shadowSamples == 0) diff = 1.0;
  for(uint j = 0; j < shadowSamples; j++){

    if(!bvhTraversal(s, v))
        diff += increase;

    float rand1 = randm(vec2(j * gl_GlobalInvocationID.x, gl_GlobalInvocationID.y));
    float rand2 = randm(vec2(j * gl_GlobalInvocationID.y, gl_GlobalInvocationID.x));
    float rand3 = randm(vec2(gl_GlobalInvocationID.y, j * gl_GlobalInvocationID.x));
    s.direction = normalize(vec3(focusPoint.x + 0.09 * rand1,
                                 focusPoint.y + 0.09 * rand2,
                                 focusPoint.z + 0.09 * rand3) - s.origin);
  }

  s.direction = s.origin + col.normal;
  focusPoint = s.origin + s.direction;
  increase = 0.7 / indirectSamples;

  for(uint j = 0; j < indirectSamples; j++){

    if(bvhTraversal(s, v)){
      indirect += increase * v.color / v.dist;
    }

    float rand1 = randm(vec2(j * gl_GlobalInvocationID.x, gl_GlobalInvocationID.y));
    float rand2 = randm(vec2(j * gl_GlobalInvocationID.y, gl_GlobalInvocationID.x));
    float rand3 = randm(vec2(gl_GlobalInvocationID.y, j * gl_GlobalInvocationID.x));
    s.direction = normalize(vec3(focusPoint.x + 0.8529 * rand1,
                                 focusPoint.y + 0.8529 * rand2,
                                 focusPoint.z + 0.8529 * rand3) - s.origin);
  }

  s.direction = light_dir;
  increase = 1.0 / aoSamples;
  amb = 0.0;

  if(aoSamples == 0)
    amb = 0.3;

  for(uint j = 0; j < aoSamples; j++){

    if(bvhTraversal(s, v)){
      amb += v.dist * increase * 0.1;
    }
    else{
      amb += 0.3 * increase;
    }

    float rand1 = randm(vec2(j * gl_GlobalInvocationID.x, gl_GlobalInvocationID.y));
    float rand2 = randm(vec2(j * gl_GlobalInvocationID.y, gl_GlobalInvocationID.x));
    float rand3 = randm(vec2(gl_GlobalInvocationID.y, j * gl_GlobalInvocationID.x));
    s.direction = normalize(vec3(focusPoint.x + 0.79 * rand1,
                                 focusPoint.y + 0.79 * rand2,
                                 focusPoint.z + 0.79 * rand3) - s.origin);
  }

  col.color += 0.25 * indirect;

  diff *= max(0.1, dot(light_dir, col.normal));
  spec *= pow(max(dot(view_dir, reflect_dir), 0), 4);

  return (amb + (0.7 * diff) + (0.5 * spec)) * normalize(col.color);

}

/* Ray triangle intersection (Moller-Trumbore algorithm)
*
* ray - input ray
* tr - input triangle
* inter - description of point of potential collision
*
* return true - collision occurs, false - no collision
*/
bool rayTriangleIntersection(Ray ray, Triangle tr, out CollisionPoint inter){

	vec3 e1 = tr.pos_b - tr.pos_a;
	vec3 e2 = tr.pos_c - tr.pos_a;

	vec3 s1 = cross(ray.direction, e2);

	float div = dot(s1, e1);
	if(div < 1e-7) return false;

	float invdiv = 1.0 / div;
	vec3 dis = ray.origin - tr.pos_a;

	float u = dot(dis, s1) * invdiv;

  if (u < 0.0 || u > 1.0)
    return false;

  vec3 qv = cross(dis, e1);
	float v = dot(ray.direction, qv) * invdiv;

  if (v < 0.0 || u + v > 1.0)
    return false;


  float t = dot(e2, qv) * invdiv;
  if(t < 1e-7)
    return false;

  // Collision point properties
	inter.dist = t;
	inter.position = ray.origin + t * ray.direction;
	inter.normal = tr.nor_a;
	inter.color = materials[tr.mat_id].diffuseCol;
  inter.metalness = materials[tr.mat_id].metalness;
  inter.roughness = materials[tr.mat_id].roughness;

  vec3 bc;
  barycentricInterpolation(tr, inter.position, bc);
  inter.normal = (bc.x * tr.nor_a) + (bc.y * tr.nor_b) + (bc.z * tr.nor_c);

  inter.uvs = (bc.x * tr.uv_a) + (bc.y * tr.uv_b) + (bc.z * tr.uv_c);
  inter.color = vec3(texture(materials[tr.mat_id].diffuseTex, inter.uvs));
  if(inter.color.r == 0.0 && inter.color.g == 0.0 && inter.color.b == 0.0)
    inter.color = materials[tr.mat_id].diffuseCol;
	//inter.refractionIndex = materials[tr.mat_id].refIndex;
	//inter.metalness = materials[tr.mat_id].metalness;

	return true;
}

/* Ray AABB intersection
*
* minbb - minimal bounding box coordinates
* maxbb - maximal bounding box coordinates
* r - input ray
* t - distance of collision
*
* return true - collision occurs, false - no collision
*/
bool boxTest(vec3 minbb, vec3 maxbb, Ray r, inout float t, inout float tbegin) {

	vec3 invdir = 1.0f / r.direction;

	float t1 = (minbb.x - r.origin.x) * invdir.x;
	float t2 = (maxbb.x - r.origin.x) * invdir.x;

	float tmin = min(t1, t2);
	float tmax = max(t1, t2);

	t1 = (minbb.y - r.origin.y) * invdir.y;
	t2 = (maxbb.y - r.origin.y) * invdir.y;

	tmin = max(tmin, min(min(t1, t2), tmax));
	tmax = min(tmax, max(max(t1, t2), tmin));

	t1 = (minbb.z - r.origin.z) * invdir.z;
	t2 = (maxbb.z - r.origin.z) * invdir.z;

	tmin = max(tmin, min(min(t1, t2), tmax));
	tmax = min(tmax, max(max(t1, t2), tmin));

	return tmax > max(tmin, 0.0f);

}


/* Ray BVH traversal
*
* r - input ray
* c - output collision point
*
* return true if found some collision with any primitive, else it returns false
*/
bool bvhTraversal(Ray r, out CollisionPoint c){

  int found = -1;
  int top = 0;
  long lstack = 0;
  long rstack = 0;
  CollisionPoint hitPoint;
  float closest = 10000.0;
  bool res = false;

  // Traversal loop
  while (top != -1) {

    Node n = tree[top];
  	bool backtrack = false;

    heat += 0.001f;

    // Leaf nodes - ray-triangle intersection tests
  	if (n.first != -1 || n.last != -1) {

      // GPU BVH leaf node
      if(bvhType == 1){

       if(n.first != -1){

          if(rayTriangleIntersection(r, data[indices[n.first]], hitPoint)){

            if(hitPoint.dist < closest){
              c = hitPoint;
              closest = hitPoint.dist;
              res = true;
            }

          }

        }

        if(n.last != -1){

          if(rayTriangleIntersection(r, data[indices[n.last]], hitPoint)){

              if(hitPoint.dist < closest){
                c = hitPoint;
                closest = hitPoint.dist;
                res = true;
              }

            }

          }

          if(n.first != -1 && n.last != -1)
            backtrack = true;

        }

        // CPU BVH leaf node
        else{

        for(int i = n.first; i <= n.last; i++){

          if(rayTriangleIntersection(r, data[i], hitPoint)){

            if(hitPoint.dist < closest){
              c = hitPoint;
              closest = hitPoint.dist;
              res = true;
            }

          }

        }

  		  backtrack = true;
      }
  	}

    // Inner node
  	if (n.left != -1 || n.right != -1) {

  		Node lf = tree[n.left];
  		Node rg = tree[n.right];

      float t1, t2;
  		bool r1 = false, r2 = false;
      float tq;

  		if (n.left != -1) r1 = boxTest(vec3(lf.min), vec3(lf.max), r, tq, t1);
  		if (n.right != -1) r2 = boxTest(vec3(rg.min), vec3(rg.max), r, tq, t2);

  		if (r1 && r2) {

  			if (t1 <= t2) {
  				top = n.left;
  				lstack = (lstack | 1) << 1;
  				rstack <<= 1;
  			}
  			else {
  				top = n.right;
  				rstack = (rstack | 1) << 1;
  				lstack <<= 1;
  			}

  		}

  		else if (r1) {
  			top = n.left;
  			lstack <<= 1;
  			rstack <<= 1;
  		}

  		else if (r2) {
  			top = n.right;
  			lstack <<= 1;
  			rstack <<= 1;
  		}

  		else {
  			backtrack = true;
  		}

  	}

    // Backtrack in BVH structure
  	if (backtrack) {

  		bool f = false;

  		while (lstack != 0 || rstack != 0) {
  			n = tree[top];

        if ((lstack & 1) != 0) {
  				// visit right node
  				top = n.right;
  				lstack &= ~1;
  				lstack <<= 1;
  				rstack <<= 1;
  				f = true;
  				break;
  			}

  			else if ((rstack & 1) != 0) {
  				// visit left node
  				top = n.left;
  				rstack &= ~1;
  				lstack <<= 1;
  				rstack <<= 1;
  				f = true;
  				break;
  			}

  			top = n.parent;
  			lstack >>= 1;
  			rstack >>= 1;

  		}

  		if (!f)
        break;

  	}

  }

  return res;

}

/*
* Ray tracing function
*
* r - traced ray
*/
vec4 rayTrace(Ray r){

  vec4 pixel = vec4(1);
  uint depth = 0;
  float refIndex = 1.0;
  CollisionPoint cp;
  vec3 fres;
  float energy = 1.0;
  vec4 background = vec4(0.9);

  while(depth < MAX_DEPTH){

    if(bvhTraversal(r, cp)){
      pixel *= vec4(energy * computeLighting(cp, fres.x), 1.0);
    }

    else
      return pixel * background;//1.4 * pixel;// * vec4(0.0095);

    energy *= cp.metalness / cp.roughness;
    refIndex *= 1.0 + cp.roughness;

    if(energy < 0.4)
      return pixel;

    if(cp.metalness > 0.5){
      r.origin = cp.position + cp.normal * EPS;
      r.direction = reflect(r.direction, cp.normal);
    }

    else{
      r.origin = cp.position - cp.normal * EPS;
      r.direction = refract(r.direction, cp.normal, refIndex);
    }

    depth += 1;
  }

  return pixel;
}

void main(){

	vec2 uv = vec2(float(gl_GlobalInvocationID.x) / width, float(gl_GlobalInvocationID.y) / height);
	vec3 dir = screen_plane[2] + (uv.x * normalize(screen_plane[0])) + (uv.y * normalize(screen_plane[1]));

	Ray r;
	r.origin = view_pos;
	r.direction = normalize(dir - view_pos);
	r.energy = 1.0f;

	vec4 pixel = vec4(0.0f);

  pixel += rayTrace(r);
  pixel = vec4(pixel.x, pixel.y, pixel.z, 1.0f);

  if(renderMode == HEATMAP) {
    pixel = vec4(vec3(heat), 1.0f);
  }

	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

	imageStore(img_output, pixel_coords, pixel);

}
