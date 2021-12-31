shader_type spatial;
render_mode skip_vertex_transform;

uniform int coneCount : hint_range(0, 30) = 30;

//This shader can display up to 30 cones (represented by 30 4d vectors) 
// Each group of 4 represents the xyz coordinates of the cone direction
// vector in model space and the fourth element represents radius

uniform float coneSequence[120]; // 4 * 30

varying vec3 vertModelNormal;

bool isInInterConePath(in vec3 normalDir, in vec4 tangent1, in vec4 cone1, in vec4 tangent2, in vec4 cone2) {
	vec3 c1xc2 = cross(cone1.xyz, cone2.xyz);		
	float c1c2dir = dot(normalDir, c1xc2);
		
	if(c1c2dir < 0.0) { 
		vec3 c1xt1 = cross(cone1.xyz, tangent1.xyz); 
		vec3 t1xc2 = cross(tangent1.xyz, cone2.xyz);	
		float c1t1dir = dot(normalDir, c1xt1);
		float t1c2dir = dot(normalDir, t1xc2);
		
	 	return (c1t1dir > 0.0 && t1c2dir > 0.0); 
			
	}else {
		vec3 t2xc1 = cross(tangent2.xyz, cone1.xyz);	
		vec3 c2xt2 = cross(cone2.xyz, tangent2.xyz);	
		float t2c1dir = dot(normalDir, t2xc1);
		float c2t2dir = dot(normalDir, c2xt2);
		
		return (c2t2dir > 0.0 && t2c1dir > 0.0);
	}	
	return false;
}

//determines the current draw condition based on the desired draw condition in the setToArgument
// -3 = disallowed entirely; 
// -2 = disallowed and on tangentCone boundary
// -1 = disallowed and on controlCone boundary
// 0 =  allowed and empty; 
// 1 =  allowed and on controlCone boundary
// 2  = allowed and on tangentCone boundary
int getAllowabilityCondition(in int currentCondition, in int setTo) {
	if((currentCondition == -1 || currentCondition == -2)
		&& setTo >= 0) {
		return currentCondition *= -1;
	} else if(currentCondition == 0 && (setTo == -1 || setTo == -2)) {
		return setTo *=-2;
	}  	
	return max(currentCondition, setTo);
}

//returns 1 if normalDir is beyond (cone.a) radians from cone.rgb
//returns 0 if normalDir is within (cone.a + boundaryWidth) radians from cone.rgb 
//return -1 if normalDir is less than (cone.a) radians from cone.rgb
int isInCone(in vec3 normalDir, in vec4 cone, in float boundaryWidth) {
	float arcDistToCone = acos(dot(normalDir, cone.rgb));
	if(arcDistToCone > (cone.a+(boundaryWidth/2.))) {
		return 1; 
	}
	if(arcDistToCone < cone.a-(boundaryWidth/2.)) {
		return -1;
	}
	return 0;
} 

//returns a color corresponding to the allowability of this region, or otherwise the boundaries corresponding 
//to various cones and tangentCone 
vec4 colorAllowed(in vec3 normalDir,  in int coneCounts, in float boundaryWidth, in vec4 color) {
	normalDir = normalize(normalDir);
	int currentCondition = -3;
	
	if(coneCounts == 1) {
		vec4 cone = vec4(coneSequence[0], coneSequence[1], coneSequence[3], coneSequence[4]);
		int inCone = isInCone(normalDir, cone, boundaryWidth);
		if (inCone == 0) {
			inCone = -1;
		} else {
			if (inCone < 0) {
				inCone = 0;
			} else {
				inCone = -3;
			}
		}
		currentCondition = getAllowabilityCondition(currentCondition, inCone);
	} else {
		for(int i=0; i<coneCounts-1; i+=3) {
			
			int idx = i*3; 
			// FIXME: fire 2021-12-28 The indexing is wrong here.
			vec4 cone1 = vec4(
				coneSequence[coneCount * 4 + (idx * 4) + 0], 
				coneSequence[coneCount * 4 + (idx * 4) + 1], 
				coneSequence[coneCount * 4 + (idx * 4) + 2],
				coneSequence[coneCount * 4 + (idx * 4) + 3]);
			vec4 tangent1 = vec4(
				coneSequence[coneCount * 4 + ((idx + 0) * 4) + 0],
				coneSequence[coneCount * 4 + ((idx + 1) * 4) + 1],
				coneSequence[coneCount * 4 + ((idx + 2) * 4) + 2],
				coneSequence[coneCount * 4 + ((idx + 3) * 4) + 3]
			);
			vec4 tangent2 = vec4(
				coneSequence[coneCount * 4 + ((idx+2) * 4) + 0],
				coneSequence[coneCount * 4 + ((idx+2) * 4) + 1],
				coneSequence[coneCount * 4 + ((idx+2) * 4) + 2],
				coneSequence[coneCount * 4 + ((idx+2) * 4) + 3]
			);
			vec4 cone2 = vec4(
				coneSequence[coneCount * 4 + ((idx+3) * 4) + 0],
				coneSequence[coneCount * 4 + ((idx+3) * 4) + 1],
				coneSequence[coneCount * 4 + ((idx+3) * 4) + 2],
				coneSequence[coneCount * 4 + ((idx+3) * 4) + 3]
			);
										
			int inCone1 = isInCone(normalDir, cone1, boundaryWidth);
			if (inCone1 == 0){
				inCone1 = -1;
			} else {
				if(inCone1 < 0) {
					inCone1 = 0;
				} else {
					inCone1 = -3;
				}
			}
			currentCondition = getAllowabilityCondition(currentCondition, inCone1);
				
			int inCone2 = isInCone(normalDir, cone2, boundaryWidth);
			if (inCone2 == 0) {
				inCone2 = -1;
			} else {
				if ( inCone2  < 0) {
					inCone2 = 0;
				} else {
					inCone2 = -3;
				}
			}
			currentCondition = getAllowabilityCondition(currentCondition, inCone2);
		
			int inTan1 = isInCone(normalDir, tangent1, boundaryWidth); 
			int inTan2 = isInCone(normalDir, tangent2, boundaryWidth);
			
			if( inTan1 < 1 || inTan2  < 1) {			
				inTan1 =  inTan1 == 0 ? -2 : -3;
				currentCondition = getAllowabilityCondition(currentCondition, inTan1);
				inTan2 =  inTan2 == 0 ? -2 : -3;
				currentCondition = getAllowabilityCondition(currentCondition, inTan2);
			} else {				 
				bool inIntercone = isInInterConePath(normalDir, tangent1, cone1, tangent2, cone2);
				int interconeCondition = inIntercone ? 0 : -3; 
				currentCondition = getAllowabilityCondition(currentCondition, interconeCondition);					
			}
		}
	}
	vec4 result = color;
	if (currentCondition != 0) {
		float onTanBoundary = abs(currentCondition) == 2 ? 0.3 : 0.0; 
		float onConeBoundary = abs(currentCondition) == 1 ? 0.3 : 0.0;
		result += vec4(0.0, onConeBoundary, onTanBoundary, 1.0);
	} else {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	return result;
}

void vertex() {
	vertModelNormal = normalize((MODELVIEW_MATRIX * vec4(NORMAL, 0.0)).xyz);
}

void fragment() {
	vec4 colorAllowed = colorAllowed(vertModelNormal, coneCount, 0.02, COLOR);  
	if (colorAllowed.a == 0.0) {
			discard;
	}
	colorAllowed += (colorAllowed + fwidth(colorAllowed)); 
	colorAllowed /= 2.0;
	vec3 lightCol = vec3(1.0,0.8,0.0);
	float gain = NORMAL.z < 0.0 ? -0.3 : 0.5;
	ALBEDO = colorAllowed.rgb;
	ALPHA = colorAllowed.a;
}