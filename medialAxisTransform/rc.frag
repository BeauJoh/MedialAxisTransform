uniform float stepSize;
uniform sampler3D gradientVol; //3DTexture containing gradient direction and magnitude
uniform sampler3D intensityVol; //3DTexture containing brightness
uniform sampler2D backFace;
uniform sampler1D TransferTexture;
uniform float viewWidth;
uniform float viewHeight;
uniform float edgeThresh, edgeExp;
uniform float DitherRay; //0.0 for none, 1.0 for 1 voxel stochastic jitter 
uniform int showGradient;
uniform float isRGBA; //if 1, then intensityVol is RGBA texture, otherwise scalar A texture
uniform float boundExp;    //Contribution of boundary enhancement calculated opacity and scaling exponent
uniform vec3 clearColor;

void main() {
	// get normalized pixel coordinate in view port (e.g. [0,1]x[0,1])
	vec2 pixelCoord = gl_FragCoord.st;
	pixelCoord.x /= viewWidth;
	pixelCoord.y /= viewHeight;	
	// starting position of the ray is stored in the texture coordinate
	vec4 start = gl_TexCoord[1];
	vec4 backPosition = texture2D(backFace,pixelCoord);
	vec3 dir = vec3(0.0,0.0,0.0);
	dir.x = backPosition.x - start.x;
	dir.y = backPosition.y - start.y;
	dir.z = backPosition.z - start.z;
	float len = length(dir.xyz);
	dir = normalize(dir);
	vec3 deltaDir = dir * stepSize;
	vec3 samplePos = start.xyz;
	vec4 colorSample,gradientSample,colAcc = vec4(0.0,0.0,0.0,0.0);
	float alphaAcc = 0.0;
	float lengthAcc = 0.0;
	float alphaSample;
	float edgeVal;
	float dotView;
	 //We need to calculate the ray's starting position. We add a random
	//fraction of the stepsize to the original starting point to dither the output
  	float random = DitherRay* fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453); 
	samplePos  = samplePos + deltaDir* (random);
	for(int i = 0; i < 450; i++) {
		if (showGradient < 1)
		{

			if (isRGBA > 0.0)
			{
				colorSample = texture3D(intensityVol,samplePos);
			} else {
				colorSample.a = texture3D(intensityVol,samplePos).a;
				colorSample= texture1D(TransferTexture, colorSample.a).rgba;
			}
		} else {
		colorSample = texture3D(gradientVol,samplePos);
		}
		//colorSample = texture3D(gradientVol,samplePos);//<-this allows you to directly view gradients
		//alphaSample = 1.0-pow((1.0 - colorSample.a), stepSize);
		if (edgeThresh < 1.0 || boundExp > 0.0) 
		{
		  gradientSample= texture3D(gradientVol,samplePos);
		  if (edgeThresh < 1.0)
		  {
			//This texture contains the normalized, scaled gradient in the rgb channels
			//and the voxel's density in the alpha channel
			//We have to scale the gradient between -1 and 1
			gradientSample.rgb = gradientSample.rgb*2.0 - 1.0;
			//Calculate the angle between the viewer and the gradient in eye space
			
			float dotView = dot(dir, gradientSample.rgb);
			//If edge highlighting is toggled we darken the voxel based on its perpendicularity
			//to the viewing direction.
			//We make very perpendicular voxels black, and lesser perpendicular voxels
			//decreasingly dark until the value is below the threshold, when no
			//enhancement is performed.
			edgeVal = pow(1.0-abs(dotView),edgeExp);
			edgeVal = edgeVal * pow(gradientSample.a,0.3);
	    		//if(edgeVal >= edgeThresh && !degenerate)
	    		if (edgeVal >= edgeThresh)
	    		{ 
				
				colorSample.rgb = mix(colorSample.rgb, vec3(0.0,0.0,0.0), pow((edgeVal-edgeThresh)/(1.0-edgeThresh),4.0));
			}
		  }
		  if (boundExp > 0.0)
			colorSample.a = colorSample.a * pow(gradientSample.a,boundExp);
		}
		colorSample.rgb *= colorSample.a;
		
		alphaSample = 1.0-pow((1.0 - colorSample.a), stepSize);
		//accumulate color
		colAcc= (1.0 - colAcc.a) * colorSample + colAcc;
		alphaAcc += alphaSample;
		samplePos += deltaDir;
		lengthAcc += stepSize;
		// terminate if opacity > 1 or the ray is outside the volume
		if ( lengthAcc >= len || alphaAcc > 0.95 )
			break;
	}

	colAcc.rgb = mix(clearColor,colAcc.rgb,colAcc.a);
	gl_FragColor = colAcc;
}