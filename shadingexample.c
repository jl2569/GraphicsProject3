

double fang(Object* light , Object* object){
	if (light->direction[0] == 0 && light->direction[1] == 0 && light->direction[2] == 0 ){
		return 1;
	}
	double final ;
	if (strcmp(object->name,"plane") == 0){
		final = (object->plane.center[0] * light->light.center[0])+ (object->plane.center[1] * light->light.center[1])+ (object->plane.center[2] * light->light.center[2]);
	}else if (strcmp(object->name,"sphere") == 0){
		final = (object->sphere.center[0] * light->light.center[0])+ (object->sphere.center[1] * light->light.center[1])+ (object->sphere.center[2] * light->light.center[2]);
	}
	if (acos(final) > light->light.angulara0){
		return 0 ;
	}else{
		return  pow(final ,light->light.angulara0 );
	}
}

double frad(Object* light, double d){
	double eq  = light->light.radiala0 + light->light.radiala1*d +light->light.radiala2*pow(d,2);
	return 1/eq;
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

    double* color = malloc(sizeof(double)*3);
    color[0] = 0; // ambient_color[0];
    color[1] = 0; // ambient_color[1];
    color[2] = 0; // ambient_color[2];

    for (int j=0; light[j] != NULL; j+=1) {
      // Shadow test
      double Ron[3] = {best_t * Rd[0] + Ro[0],
		  best_t * Rd[1] + Ro[1],
		  best_t * Rd[2] + Ro[2]
	  };
      double Rdn[3] = {light_position[0] - Ron[0],
		light_position[1] - Ron[1],
		light_position[2] - Ron[2]
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
	if (strcmp(objects[closest_shadow_object]->name,"plane") == 0){
		N = {objects[closest_shadow_object]->plane.normal[0],
			objects[closest_shadow_object]->plane.normal[1],
			objects[closest_shadow_object]->plane.normal[2]
		};// plane
	}else if(strcmp(objects[closest_shadow_object]->name,"sphere") == 0){
	N = {Ron[0] - objects[closest_shadow_object]->sphere.center[0], 
	Ron[1] - objects[closest_shadow_object]->sphere.center[1],
	Ron[2] - objects[closest_shadow_object]->sphere.center[2]
	};// sphere
	}
	L = Rdn; // light_position - Ron;
	normalize(Ron);
	R = light_position-2(light_position[0]*Ron[0] + light_position[1]*Ron[1] + light_position[2]*Ron[2])*Ron;
	V = Rd;
	double diffuse[3] =  {1+N*L*objects[closest_shadow_object]->sphere.diffuse_color[0],
	1+N*L*objects[closest_shadow_object]->sphere.diffuse_color[1],
	1+N*L*objects[closest_shadow_object]->sphere.diffuse_color[2],
	}; // uses object's diffuse color
	double specular[3] = {pow(R*V,N)*objects[closest_shadow_object]->sphere.specular_color[0],
	objects[closest_shadow_object]->sphere.diffuse_color[1],
	objects[closest_shadow_object]->sphere.diffuse_color[2]
	}; // uses object's specular color
	color[0] += frad() * fang() * (diffuse[0] + specular[0]);
	color[1] += frad() * fang() * (diffuse[1] + specular[1]);
	color[2] += frad() * fang() * (diffuse[2] + specular[2]);
      }
    }
    // The color has now been calculated

    buffer[...].r = (unsigned char)(255 * clamp(color[0]));
    buffer[...].g = (unsigned char)(255 * clamp(color[1]);
    buffer[...].b = (unsigned char)(255 * clamp(color[2]);


    }
    //printf("\n");
  }
