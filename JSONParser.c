#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// Plymorphism in C

typedef struct {
  int kind; // 0 = plane, 1 = sphere 2 = plane 3=light
  char* name;
  double color[3];
  double specular_color[3];
  double diffuse_color[3];
  union {
    struct {
      double width;
      double height;
    } camera;
    struct {	
      double center[3];
      double radius;
    } sphere;
    struct {
	  double center[3];
      double normal[3];
    } plane;
	struct {
	  double center[3];
      double direction[3];
	  double radiala2;
	  double radiala1;
	  double radiala0;
	  double angulara0;
	  double theta;
    } light;
  };
} Object;

static inline double sqr(double v) {
  return v*v;
}


static inline void normalize(double* v) {
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}

// function that looks for the plane on the image by doing plane intersection and determines if the plane is in the spot of the image
double plane_intersection(double* Ro, double* Rd,
			     double* C, double* n) {
  normalize(n);
  double a = (n[0]* Rd[0])+(n[1]* Rd[1])+(n[2]* Rd[2]);

  if(fabs(a) < .0001) {
    return -1;
  }
  
  double b[3];
  for (int i=0; i<=2;i++){
	  b[i] = C[i]-Ro[i];
  }
  double d = (b[0]* n[0])+(b[1]* n[1])+(b[2]* n[2]) ;

  double t = d/a;

  if (t < 0.0) {
	
    return -1;
  }

  return t;
}
// function that looks for the sphere on the image by doing sphere intersection and determines if the sphere is in the spot of the image
double sphere_intersection(double* Ro, double* Rd,
			     double* C, double r) {
  double a = (sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]));
  double b = (2 * (Rd[0] * (Ro[0] - C[0]) + Rd[1] * (Ro[1] - C[1]) + Rd[2] * (Ro[2] - C[2])));
  double c = sqr(Ro[0]- C[0]) + sqr(Ro[1]- C[1]) +sqr(Ro[2]- C[2]) - sqr(r);
  double det = sqr(b) - 4 * a * c;
  if (det < 0) return -1;
  det = sqrt(det);
  
  double t0 = (-b - det) / (2*a);
  if (t0 > 0) return t0;

  double t1 = (-b + det) / (2*a);
  if (t1 > 0) return t1;

  return -1;
}

double* diffuse(double* N, double* L , Object* object,double* final){
	double test = N[0]*L[0]+N[1]*L[1]+N[2]*L[2];
	if (test<0){
		return 0;
	} 
	final[0] = test*object->diffuse_color[0];
	final[1] = test*object->diffuse_color[1];
	final[2] = test*object->diffuse_color[2];
	normalize(final);
	return final;
	}
	
double* specular(double* R, double* V ,double* N, double* L , Object* object ,double* final){
	double test = N[0]*L[0]+N[1]*L[1]+N[2]*L[2];
	double cest = pow((R[0]*V[0]+R[1]*V[1]+R[2]*V[2]),20);
	if (test == 0 && cest == 0){
		return 0;
	}
	final[0]=cest*object->specular_color[0];
	final[1]=cest*object->specular_color[1];
	final[2]=cest*object->specular_color[2];
	
	return final;	
}	
	
double fang(Object* light ,double t ){
	if (light->light.direction[0] == 0 && light->light.direction[1] == 0 && light->light.direction[2] == 0 ){
		return 1;
	}
	double vect[3] = { t-light->light.center[0],
	t-light->light.center[1],
	t-light->light.center[2]}; 
	normalize(vect);
	double final = (vect[1]*light->light.direction[0])+(vect[2]*light->light.direction[1])+(vect[3]*light->light.direction[2]);
	if (final < 0){
		return 0 ;
	}else{
		return  pow(final ,light->light.angulara0);
	}
}

double frad(Object* light, double d){
	if (d == INFINITY){
		return 1;
	}else{
		double eq  = light->light.radiala0 + light->light.radiala1*d +light->light.radiala2*pow(d,2);
		return 1/eq;
	}
}

double distance(double* a, double* b){
	normalize(a);
	normalize(b);
	return sqrt((((a[0] * b[0]) * (a[0] * b[0])) +((a[1] * b[1]) * (a[1] * b[1])) + ((a[2] * b[2]) * (a[2] * b[2]))));
}


int line = 1;

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json) {
  int c = fgetc(json);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* json, int d) {
  int c = next_c(json);
  if (c == d) return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);    
}


// skip_ws() skips white space in the file.
void skip_ws(FILE* json) {
  int c = next_c(json);
  while (isspace(c)) {
    c = next_c(json);
  }
  ungetc(c, json);
}


// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json) {
  char buffer[129];
  int c = next_c(json);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }  
  c = next_c(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);      
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);      
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = next_c(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

double next_number(FILE* json) {
  double value;
  fscanf(json, "%lf", &value);
  //if (value == EOF) {
  //   fprintf(stderr, "Error: Expected a number but not found.\n");
  //   exit(1);
  //  }
  return value;
}

double* next_vector(FILE* json) {
  double* v = malloc(3*sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  v[0] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[1] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[2] = next_number(json);
  skip_ws(json);
  expect_c(json, ']');
  return v;
}
// sets the single value elements to their apporiate type and object
Object** valuesetter(int type,char* key ,double value,Object** objects,int elements ){
	if (type == 0){
		if((strcmp(key,"width")==0)){
			objects[elements]->camera.width = value;
			return objects;
		}else if ((strcmp(key,"height")==0)){
			objects[elements]->camera.height = value;
			return objects;
		}else{
			fprintf(stderr,"Error:camera does not support %s\n",key);
			exit(1);
		}
		
	}else if (type == 1) {
		if ((strcmp(key, "radius") == 0)){
			objects[elements]->sphere.radius = value;
			return objects;
		}else{
			fprintf(stderr,"Error:sphere does not support %s\n",key);
			exit(1);
		}
	}else if (type == 3) {
		if ((strcmp(key, "radial-a0") == 0)){
			objects[elements]->light.radiala0 = value;
			return objects;
		}else if ((strcmp(key, "radial-a1") == 0)){
			objects[elements]->light.radiala1 = value;
			return objects;
		}else if ((strcmp(key, "radial-a2") == 0)){
			objects[elements]->light.radiala2 = value;
			return objects;
		}else if ((strcmp(key, "angular-a0") == 0)){
			objects[elements]->light.angulara0 = value;
			return objects;
		}else if ((strcmp(key, "theta") == 0)){
			objects[elements]->light.theta = value;
			return objects;
		}else{
			fprintf(stderr,"Error:light does not support %s\n",key);
			exit(1);
		}
	}else{
		fprintf(stderr,"Error: Key %s is not supported\n",type);
		exit(1);
	}
}
// sets the vector value elements to their apporiate type and object
Object** vectorsetter(int type,char* key ,double* value,Object** objects,int elements ){
	if (type == 1){
		if ((strcmp(key, "color") == 0)){
			for (int i=0;i<=2;i++){
				if (value[i] >1 || value[i] < 0){
					fprintf(stderr,"Error:value in color for sphere is not inbetween 0 and 1\n");
					exit(1);
				}else{
			   objects[elements]->color[i] = value[i];
				}
			}
			return objects;
		}else if ((strcmp(key, "diffuse_color") == 0)){
			for (int i=0;i<=2;i++){
				if (value[i] >1 || value[i]< 0){
					fprintf(stderr,"Error:value in diffuse_color for sphere is not inbetween 0 and 1\n");
					exit(1);
				}else{
			   objects[elements]->diffuse_color[i] = value[i];
				}
			}
			return objects;
		}else if ((strcmp(key, "specular_color") == 0)){
			for (int i=0;i<=2;i++){
				if (value[i] >1 || value[i]< 0){
					fprintf(stderr,"Error:value in specular_color for sphere is not inbetween 0 and 1\n");
					exit(1);
				}else{
			   objects[elements]->specular_color[i] = value[i];
				}
			}
			return objects;
		}else if ((strcmp(key, "position") == 0)){
			for (int i=0;i<=2;i++){
			    objects[elements]->sphere.center[i] = value[i];
			}
			return objects;
		}else{
			fprintf(stderr,"Error:sphere does not support %s\n",key);
			exit(1);
		}
	}else if (type == 2) {
		if ((strcmp(key, "color") == 0)){			
		for (int i=0;i<=2;i++){
				if (value[i] >1 || value[i]< 0){
					fprintf(stderr,"Error:value in color for plane is not inbetween 0 and 1\n");
					exit(1);
				}else{
			    objects[elements]->color[i] = value[i];
				}
			}
			return objects;
		}else if ((strcmp(key, "position") == 0)){
			for (int i=0;i<=2;i++){
			    objects[elements]->plane.center[i] = value[i];
			}
			return objects;
		}else if ((strcmp(key, "normal") == 0)){
			for (int i=0;i<=2;i++){
			    objects[elements]->plane.normal[i] = value[i];
			}
			return objects;
		}else if ((strcmp(key, "diffuse_color") == 0)){
			for (int i=0;i<=2;i++){
				if (value[i] >1 || value[i]< 0){
					fprintf(stderr,"Error:value in diffuse_color for plane is not inbetween 0 and 1\n");
					exit(1);
				}else{
			   objects[elements]->diffuse_color[i] = value[i];
				}
			}
			return objects;
		}else if ((strcmp(key, "specular_color") == 0)){
			for (int i=0;i<=2;i++){
				if (value[i] >1 || value[i]< 0){
					fprintf(stderr,"Error:value in specular_color for plane is not inbetween 0 and 1\n");
					exit(1);
				}else{
			   objects[elements]->specular_color[i] = value[i];
				}
			}
			return objects;
		}else{
			fprintf(stderr,"Error:plane does not support %s\n",key);
			exit(1);
		}
		
	}else if (type == 3) {
		if ((strcmp(key, "color") == 0)){			
			for (int i=0;i<=2;i++){
				objects[elements]->color[i] = value[i];
			}
			return objects;
		}else if ((strcmp(key, "position") == 0)){
			for (int i=0;i<=2;i++){
			    objects[elements]->light.center[i] = value[i];
			}
			return objects;
		}else if ((strcmp(key, "direction") == 0)){
			for (int i=0;i<=2;i++){
			    objects[elements]->light.direction[i] = value[i];
			}
			return objects;
		}else{
			fprintf(stderr,"Error:light does not support %s\n",key);
			exit(1);
		}
		
	}else{
		fprintf(stderr,"Error: %s is not supported\n",type);
		exit(1);
	}
}

// reads the JSON file and figures out the all the types and sets them in to the objects
Object** read_scene(char* filename , Object** objects) {
  int c;
  int elements = 0;
  FILE* json = fopen(filename, "r"); 
  
  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }
  
  skip_ws(json);
  
  // Find the beginning of the list
  expect_c(json, '[');

  skip_ws(json);

  // Find the objects
   
  while (1) {
	int type = 0;
    c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
      fclose(json);
      return objects;
    }
    if (c == '{') {
      skip_ws(json);
    
      // Parse the object
      char* key = next_string(json);
      if (strcmp(key, "type") != 0) {
	fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
	exit(1);
      }

      skip_ws(json);

      expect_c(json, ':');

      skip_ws(json);

      char* value = next_string(json);

      if (strcmp(value, "camera") == 0) {
		  objects[elements] = malloc(sizeof(Object));
		  objects[elements]->name = "camera";
		  objects[elements]->kind = 2;
		  type = 0;
      } else if (strcmp(value, "sphere") == 0) {
		  objects[elements] = malloc(sizeof(Object));
		  objects[elements]->name = "sphere";
		  objects[elements]->kind = 1;
		  type = 1;
      } else if (strcmp(value, "plane") == 0) {
		  objects[elements] = malloc(sizeof(Object));
		  objects[elements]->name = "plane";
		  objects[elements]->kind = 0;
		  type =2;
      } else if (strcmp(value, "light") == 0) {
		  objects[elements] = malloc(sizeof(Object));
		  objects[elements]->name = "light";
		  objects[elements]->kind = 3;
		  type =3;
      } else {
	fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
	exit(1);
      }

      skip_ws(json);

      while (1) {
	// , }
	c = next_c(json);
	if (c == '}') {
	  // stop parsing this object
	  break;
	} else if (c == ',') {
	  // read another field
	  skip_ws(json);
	  char* key = next_string(json);
	  skip_ws(json);
	  expect_c(json, ':');
	  skip_ws(json);
	  if ((strcmp(key, "width") == 0) ||
	      (strcmp(key, "height") == 0) ||
	      (strcmp(key, "radius") == 0) ||
	      (strcmp(key, "radial-a0") == 0) ||
	      (strcmp(key, "radial-a1") == 0) ||
	      (strcmp(key, "radial-a2") == 0) ||
	      (strcmp(key, "angular-a0") == 0) ||
		  (strcmp(key, "theta") == 0) ) {
	    double value = next_number(json);
		valuesetter(type, key,value, objects,elements);
	  } else if ((strcmp(key, "color") == 0) ||
		     (strcmp(key, "position") == 0) ||
		     (strcmp(key, "normal") == 0) ||
	      (strcmp(key, "direction") == 0) ||
	      (strcmp(key, "diffuse_color") == 0) ||
	      (strcmp(key, "specular_color") == 0)) {
	    double* value = next_vector(json);
		vectorsetter(type, key,value,objects,elements);
	  } else {
	    fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
		    key, line);
	    //char* value = next_string(json);
	  }
	  skip_ws(json);
	} else {
	  fprintf(stderr, "Error: Unexpected value on line %d\n", line);
	  exit(1);
	}
      }
      skip_ws(json);
      c = next_c(json);
      if (c == ',') {
	// noop
	elements+= 1;
	skip_ws(json);
      } else if (c == ']') {  
	fclose(json);
	return objects;
      } else {
	fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
	exit(1);
      }
    }
  }
}

int main(int argc, char *argv[] ) {
  if(argc <= 4){
	fprintf(stderr, "Error: # of Arguments do not match number required.\n");
	exit(1);
  }
  FILE *fp;
  // checks whether or not the height and the width of the image are apporiate
  if(atoi(argv[1])<1){
	fprintf(stderr, "Error: cannot generate a image with a width less than 1\n");
	exit(1);  
  }
    if(atoi(argv[2])<1){
	fprintf(stderr, "Error: cannot generate a image with a height less than 1\n");
	exit(1);  
  }

  fp = fopen(argv[4],"w+");
  Object** objects;
  objects = malloc(sizeof(Object*)*2);
  read_scene(argv[3], objects);
  char *header = (char *)malloc(100);
  // sets the header of the image
  fputs("P3 ",fp);
  strcpy(header,argv[1]);
  fputs(header,fp);
  fputs(" ",fp);
  strcpy(header,argv[2]);
  fputs(header,fp);
  fputs(" 255\n",fp);
  
  double cx = 0;
  double cy = 0;
  
  int find = 0;
  while(strcmp(objects[find]->name,"camera") != 0){
	  find +=1;
  }
  //Sets the view of the camera
  double h= objects[find]->camera.height;
  double w = objects[find]->camera.width;
  int M = atoi(argv[2]);
  int N = atoi(argv[1]);
  double pixheight = h / M;
  double pixwidth = w / N;
  find =0;
  while(objects[find]->name != NULL){
	  find +=1;
  }
  for (int y = M; y > 0; y -= 1) {
    for (int x = 0; x < N; x += 1) {
      double Ro[3] = {0, 0, 0};
      // Rd = normalize(P - Ro)
      double Rd[3] = {
	cx - (w/2) + pixwidth * (x + 0.5),
	cy - (h/2) + pixheight * (y + 0.5),
	1
      };
      normalize(Rd);

      double best_t = INFINITY;
	  double t = 0;
	  int closest_object = 2 ;
      for (int i=0; i<find; i += 1) {
		
		switch(objects[i]->kind) {	
		case 0:
			t = plane_intersection(Ro, Rd,
				    objects[i]->plane.center,
				    objects[i]->plane.normal);
			break;
		case 1:
			if(objects[i]->sphere.radius == 0){
				t = sphere_intersection(Ro, Rd,
					objects[i]->sphere.center,
					1);
			}else{
			t = sphere_intersection(Ro, Rd,
					objects[i]->sphere.center,
					objects[i]->sphere.radius);
			}
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			// Horrible error
			exit(1);
		}
		if (t > 0 && t < best_t) {
			best_t = t;
			closest_object = i;
			
		}
      }
	int hold = 0; 
	Object** light;
	light = malloc(sizeof(Object*)*2);
	for(int k = 0; k < find ; k++){
		if (strcmp(objects[k]->name , "light")==0){
			light[hold] = objects[k];
			hold += 1;
		}
	}
	double* color = malloc(sizeof(double)*3);
    color[0] = 0; // ambient_color[0];
    color[1] = 0; // ambient_color[1];
    color[2] = 0; // ambient_color[2];
    for (int j=0; j < hold; j++) {
      // Shadow test
      double Ron[3] = {(best_t * Rd[0]) + Ro[0],
		  (best_t * Rd[1]) + Ro[1],
		  (best_t * Rd[2]) + Ro[2]
	  };
	  double light_distance = distance(Ron , light[j]->light.center);
      double Rdn[3] = {light[j]->light.center[0] - Ron[0],
		light[j]->light.center[1] - Ron[1],
		light[j]->light.center[2] - Ron[2]
	  };
    int shadow =1;
	
    for (int k=0; k<find; k ++) {
	 
	if (objects[k]->name == objects[closest_object]->name) continue;
	// 
	switch(objects[k]->kind)  {	
		case 0:
			t = plane_intersection(Ron, Rdn,
				    objects[k]->plane.center,
				    objects[k]->plane.normal);
			break;
		case 1:
			if(objects[k]->sphere.radius == 0){
				t = sphere_intersection(Ron, Rdn,
					objects[k]->sphere.center,
					1);
			}else{
			t = sphere_intersection(Ron, Rdn,
					objects[k]->sphere.center,
					objects[k]->sphere.radius);
			}
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
		}	
	if (best_t > light_distance) {
	  shadow = 0;
	  continue;
	}
      }
     if (shadow == 0 ) {
	// N, L, R, V
	double N[3];
	if (strcmp(objects[closest_object]->name,"plane") == 0){
		N[0] = objects[closest_object]->plane.normal[0];
		N[1] =	objects[closest_object]->plane.normal[1];
		N[2] =	objects[closest_object]->plane.normal[2];
		// plane
	}else if(strcmp(objects[closest_object]->name,"sphere") == 0){
		N[0] = Ron[0] - objects[closest_object]->sphere.center[0];
		
		N[1] = Ron[1] - objects[closest_object]->sphere.center[1];
		N[2] = Ron[2] - objects[closest_object]->sphere.center[2];
	// sphere
	}
	
	normalize(Ron);
	double* L = Rdn; // light_position - Ron;

	double R[3] = {light[j]->light.center[0] - 2*light[j]->light.center[0]*Ron[0],
		light[j]->light.center[1] - 2*light[j]->light.center[1]*Ron[1],
		light[j]->light.center[2] - 2*light[j]->light.center[2]*Ron[2],
	};
	double* V = Rd;
	double final[3];
	double output[3];
	normalize(V);
	diffuse(N,L,objects[closest_object],final);
	specular(R,V,N,L,objects[closest_object],output);
	color[0] += frad(light[j], best_t) * fang(light[j],best_t) * (final[0] + output[0]);
	color[1] += frad(light[j], best_t) * fang(light[j],best_t) * (final[1] + output[1]);
	color[2] += frad(light[j], best_t) * fang(light[j],best_t) * (final[2] + output[2]);
      }
    }
    // The color has now been calculated
    double temp;
		char word[1000];
		temp= color[0] *255;
		sprintf(word,"%lf ",temp);
		fputs(word,fp);
		temp= color[1] *255;
		sprintf(word," %lf ",temp);
		fputs(word,fp);
		temp= color[2] *255;
		sprintf(word,"%lf\n",temp);
		fputs(word,fp);
		
    }
    //printf("\n");
  }
  close(fp);
  return 0;
}