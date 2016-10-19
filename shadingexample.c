double* diffuse(double* N, double* L , Object* object, int take){
	double test = N[0]*L[0]+N[1]*L[1]+N[2]*L[2];
	double final[3];
	if (test<0){
		return 0;
	}
	if (strcmp(object->name,"plane") ==0){
		final = {test*objects[take]->plane.diffuse_color[0]
		test*objects[take]->plane.diffuse_color[1]
		test*objects[take]->plane.diffuse_color[2]};
	}else if (strcmp(object->name,"sphere") ==0){
		final ={test*objects[take]->sphere.diffuse_color[0]
		test*objects[take]->sphere.diffuse_color[1]
		test*objects[take]->sphere.diffuse_color[2]};
	}
	return final;
	}
	
double* specular(double* R, double* V ,double* N, double* L , Object* object, int take){
	double test = N[0]*L[0]+N[1]*L[1]+N[2]*L[2];
	double cest = R[0]*V[0]+R[1]*V[1]+R[2]*V[2];
	double final[3];
	if (test == 0 && cest == 0){
		return 0;
	}
	if (strcmp(object->name,"plane") ==0){
		final = {cest*objects[take]->plane.specular_color[0]
		cest*objects[take]->plane.specular_color[1]
		cest*objects[take]->plane.specular_color[2]};
	}else if (strcmp(object->name,"sphere") ==0){
		final ={cest*objects[take]->sphere.specular_color[0]
		cest*objects[take]->sphere.specular_color[1]
		cest*objects[take]->sphere.specular_color[2]};
	}
	return final;	
}	
	
double fang(Object* light ,double t ){
	if (light->direction[0] == 0 && light->direction[1] == 0 && light->direction[2] == 0 ){
		return 1;
	}
	double vect1 = t-light->center[0];
	double vect2 = t-light->center[1];
	double vect3 = t-light->center[2];
	double final = (vect1*light->direction[0])+(vect2*light->direction[1])+(vect3*light->direction[2]);
	
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


for (int y = 0; y < M; y += 1) {
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
	  Object* closest_object = NULL;
	  int flash = 2;
      for (int i=0; i<find; i += 1) {
		double t = 0;
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
			closest_object = objects[i];
			flash = i;
			
		}
      }
	int lightz = 0;
	int hold = 0; 
	Objects* light;
	while(objects[lightz]->name != NULL){
		if (strcmp(objects[lightz]->name , "light")==0){
			light[hold] = objects[lightz];
			hold += 1;
		}
	}
	double* color = malloc(sizeof(double)*3);
    color[0] = 0; // ambient_color[0];
    color[1] = 0; // ambient_color[1];
    color[2] = 0; // ambient_color[2];
    for (int j=0; j < hold; j+=1) {
      // Shadow test
      double Ron[3] = {best_t * Rd[0] + Ro[0],
		  best_t * Rd[1] + Ro[1],
		  best_t * Rd[2] + Ro[2]
	  };
      double Rdn[3] = {light[j]->light.position[0] - Ron[0],
		light[j]->light.position[1] - Ron[1],
		light[j]->light.position[2] - Ron[2]
	  };
      closest_shadow_object = NULL;
      for (int k=0; object[k] != NULL; k+=1) {
	if (object[k] == closest_object) continue;
	// 
	switch(objects[i]->kind) {	
		case 0:
			t = plane_intersection(Ron, Rdn,
				    objects[i]->plane.center,
				    objects[i]->plane.normal);
			break;
		case 1:
			if(objects[i]->sphere.radius == 0){
				t = sphere_intersection(Ron, Rdn,
					objects[i]->sphere.center,
					1);
			}else{
			t = sphere_intersection(Ron, Rdn,
					objects[i]->sphere.center,
					objects[i]->sphere.radius);
			}
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
		}
	if (best_t > distance_to_light) {
	  continue;
	}
      }
     if (closest_shadow_object == NULL) {
	// N, L, R, V
	
	double N[3];
	if (strcmp(objects[flash]->name,"plane") == 0){
		N = {objects[flash]->plane.normal[0],
			objects[flash]->plane.normal[1],
			objects[flash]->plane.normal[2]
		};// plane
	}else if(strcmp(objects[flash]->name,"sphere") == 0){
	N = {Ron[0] - objects[flash]->sphere.center[0], 
	Ron[1] - objects[flash]->sphere.center[1],
	Ron[2] - objects[flash]->sphere.center[2]
	};// sphere
	}
	L = Rdn; // light_position - Ron;
	normalize(Ron);
	R = light_position-2(light_position[0]*Ron[0] + light_position[1]*Ron[1] + light_position[2]*Ron[2])*Ron;
	V = Rd;
	double diffusevect[3] = diffuse(N,L,closest_object,flash)
	double specular[3]; // uses object's specular color
	color[0] += frad(light[i], best_t) * fang(light[i],best_t) * (diffusevect[0] + specular[0]);
	color[1] += frad(light[i], best_t) * fang(light[i],best_t) * (diffusevect[1] + specular[1]);
	color[2] += frad(light[i], best_t) * fang(light[i],best_t) * (diffusevect[2] + specular[2]);
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
