// ray, sphere code from https://www.shadertoy.com/view/XtBXDw

struct ray_t {
	vec3 origin;
	vec3 direction;
};

struct sphere_t {
	vec3 origin;
	float radius;
};
    
struct hit_t {
	float t;
	vec3 normal;
	vec3 origin;
};
    
#define max_dist 1e8
hit_t no_hit = hit_t(
	float(max_dist + 1e1), // 'infinite' distance
	vec3(0., 0., 0.), // normal
	vec3(0., 0., 0.) // origin
);

ray_t get_primary_ray(
	in vec3 cam_local_point,
	inout vec3 cam_origin,
	inout vec3 cam_look_at)
{
	vec3 fwd = normalize(cam_look_at - cam_origin);
	vec3 up = vec3(0, 1, 0);
	vec3 right = cross(up, fwd);
	up = cross(fwd, right);

    ray_t r;
    r.origin = cam_origin;
    r.direction = normalize(fwd + up * cam_local_point.y - right * cam_local_point.x);
	return r;
}

void intersect_sphere(
	in ray_t ray,
	in sphere_t sphere,
	inout hit_t hit
){
	vec3 rc = sphere.origin - ray.origin;
	float radius2 = sphere.radius * sphere.radius;
	float tca = dot(rc, ray.direction);
//	if (tca < 0.) return;

	float d2 = dot(rc, rc) - tca * tca;
	if (d2 > radius2)
		return;

	float thc = sqrt(radius2 - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	if (t0 < 0.) t0 = t1;
	if (t0 > hit.t)
		return;

	vec3 impact = ray.origin + ray.direction * t0;

	hit.t = t0;
	hit.origin = impact;
	hit.normal = (impact - sphere.origin) / sphere.radius;
}

/////////////////////////////////////////////////////////////////////
// TODO: Need physically correct measures. Remove these magic numbers.
//
// https://en.wikipedia.org/wiki/Luminosity
//     luminosity of the Sun which is 3.86×1026 W
//
// what???
//     sun_luminosity / (sun_dist^2 * 4pi) = 13.61839144264511 * 10^2 (Watts / meter^2)
#define SunColor              vec3(1.0, 1.0, 1.0)
#define SunIntensity          13.61839144264511
#define SunDiskMultiplier     10.0
#define MAGIC_RAYLEIGH        1.0
#define MAGIC_MIE             0.2
/////////////////////////////////////////////////////////////////////

// Animates the sun height if 0
#define FIXED_SUN_DIRECTION  0
#define SunDirection          normalize(vec3(0.0, -0.3, 1.0))

#define PI                    3.14159265359
#define FOV                   60.0

// Unit: meters
// 149,600,000 km = (1.496 * 10^11) m
#define SUN_DISTANCE          1.496e11
// 695,510 km = (6.9551 * 10^8) m
#define SUN_RADIUS            6.9551e8
// 6360 km = (6.36 * 10^6) m
#define EARTH_RADIUS          6.36e6
#define ATMOSPHERE_RADIUS     6.42e6
#define Hr                    7.994e3
#define Hm                    1.2e3

// Unit: 1 / meters
#define BetaR                 vec3(5.8e-6, 13.5e-6, 33.1e-6)
#define BetaM                 vec3(21e-6)

sphere_t atmosphere = sphere_t(vec3(0.0), ATMOSPHERE_RADIUS);

float phaseR(float cosTheta)
{
    return 3.0 / (16.0 * PI) * (1.0 + cosTheta * cosTheta);
}
float phaseM(float t)
{
    const float g = 0.76;
    float gg = g * g;
    float tt = t * t;
    float gt = g * t;
    
    float num = 3.0 * (1.0 - gg) * (1.0 + tt);
	float denom = (8.0 * PI) * (2.0 + gg) * pow(1.0 + gg - 2.0 * gt, 1.5);
    return num / denom;
}

// Radiance of sun before hitting the atmosphere
vec3 sunImage(ray_t camera, vec3 sunDir)
{
    // EARTH_RADIUS is too small compared to SUN_DISTANCE
    //float denom = exp(0.5 * (log(SUN_DISTANCE + EARTH_RADIUS) + log(SUN_DISTANCE - EARTH_RADIUS)));
    
    float threshold = asin(SUN_RADIUS / SUN_DISTANCE);
    float angle = acos(dot(camera.direction, -sunDir));
    if(angle <= threshold * SunDiskMultiplier)
    {
        return SunIntensity * SunColor;
    }
    return vec3(0.0);
}

vec3 scene(ray_t camera, vec3 sunDir)
{
    hit_t hit = no_hit;
    intersect_sphere(camera, atmosphere, hit);
    
    vec3 AtmosphereScattering = vec3(0.0);
    bool isGround = false;
    
    const int numSteps = 64;
    const int inscatSteps = 16;
    
    float mu = dot(-sunDir, camera.direction);
    vec3 Sun = SunIntensity * SunColor;
    vec3 T = vec3(0.0);
    
    vec3 P = camera.origin;
    float seg = hit.t / float(numSteps);
    vec3 P_step = camera.direction * seg;
    
    // from eye to the outer end of atmosphere
    for(int i=0; i<numSteps; ++i)
    {
        float height = length(P) - EARTH_RADIUS;
        if(height < 0.0)
        {
            isGround = true;
            break;
        }
        
        // optical depth
        T += seg * (BetaR * exp(-height / Hr));
        T += seg * (BetaM * exp(-height / Hm));
        
        // single scattering
        hit_t hit2 = no_hit;
        ray_t ray2 = ray_t(P, -sunDir);
        intersect_sphere(ray2, atmosphere, hit2);
        
        float segLight = hit2.t / float(inscatSteps);
        vec3 PL_step = ray2.direction * segLight;
        vec3 PL = P;
        
        vec3 TL = vec3(0.0);
        bool applyScattering = true;
        for(int j=0; j<inscatSteps; ++j)
        {
            float height2 = length(PL) - EARTH_RADIUS;
            if(height2 < 0.0)
            {
                applyScattering = false;
                break;
            }
            
            TL += segLight * BetaR * exp(-height2 / Hr);
        	TL += segLight * BetaM * exp(-height2 / Hm);
            
            PL += PL_step;
        }
        if(applyScattering)
        {
            TL = exp(-TL);

            vec3 SingleScattering = vec3(0.0);
            // scattering = transmittance * scattering_coefficient * phase * radiance
            SingleScattering += MAGIC_RAYLEIGH * seg * exp(-T) * (BetaR * exp(-height / Hr)) * phaseR(mu) * (TL * Sun);
            SingleScattering += MAGIC_MIE * seg * exp(-T) * (BetaM * exp(-height / Hm)) * phaseM(mu) * (TL * Sun);
            AtmosphereScattering += SingleScattering;
        }
        
        P += P_step;
    }
    
    // Just magic number
    if(isGround)
    {
        float r = 1.0 - 1.0 / (1.0 + 0.000001 * length(hit.origin - camera.origin));
        return vec3(r, 0.4, 0.2);
    }
    
    T = exp(-T);
    
    // Zero scattering
	vec3 L0 = T * sunImage(camera, sunDir);
    AtmosphereScattering += L0;
    
    return AtmosphereScattering;
}

void rotateY(inout vec3 v, float dt)
{
    float vx = v.x;
    v.x = cos(dt) * v.x + sin(dt) * v.z;
    v.z = -sin(dt) * vx + cos(dt) * v.z;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 u_res = iResolution.xy;
    vec2 aspect_ratio = vec2(u_res.x / u_res.y, 1);
	float fov = tan(radians(FOV / 2.0));
	vec2 point_ndc = fragCoord.xy / u_res.xy;
	vec3 point_cam = vec3((2.0 * point_ndc - 1.0) * aspect_ratio * fov, -1.0);
    
    vec3 eye = vec3(0.0, EARTH_RADIUS, 0.0);
	vec3 look_at = vec3(0.0, EARTH_RADIUS + 10.0, -30.0);
	ray_t eye_ray = get_primary_ray(point_cam, eye, look_at);
    
    if(iMouse.z > 0.0) rotateY(eye_ray.direction, (iResolution.x/2.0 - iMouse.x) * 0.01);

#if FIXED_SUN_DIRECTION
    vec3 sunDir = SunDirection;
#else
	float angle_min = 45.0 * (PI / 180.0);
    float angle_max = 95.0 * (PI / 180.0);
    float t = fract(iTime * 0.1);
    t = (t <= 0.5)
        ? (angle_min + (angle_max - angle_min) * t * 2.0)
        : (angle_max + (angle_min - angle_max) * (t - 0.5) * 2.0);
    vec3 sunDir = vec3(0.0, -cos(t), sin(t));
#endif
    
    vec3 finalColor = scene(eye_ray, sunDir);
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    
    // Output to screen
    fragColor = vec4(finalColor, 1.0);
}
