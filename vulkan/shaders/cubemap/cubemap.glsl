vec3 cubeCoordToWorld(ivec3 cubeCoord, vec2 cubemapSize)
{
    vec2 texCoord = vec2(cubeCoord.xy) / cubemapSize;
    switch (cubeCoord.z)
    {
    case	0: return vec3(1.0, -texCoord.yx); // posx
    case	1: return vec3(-1.0, -texCoord.y, texCoord.x); //negx
    case	2: return vec3(texCoord.x, 1.0, texCoord.y); //	posy
    case	3: return vec3(texCoord.x, -1.0, -texCoord.y); //negy
    case	4: return vec3(texCoord.x, -texCoord.y, 1.0); // posz
    case	5: return vec3(-texCoord.xy, -1.0);	// negz
    }
    return vec3(0.0);
}

float max3(vec3 val)
{
    return max(val.x, max(val.y, val.z));
}

ivec3 texCoordToCube(vec3 texCoord, vec2 cubemapSize)
{
    vec3 abst = abs(texCoord);
    texCoord /= max3(abst);

    float cubeFace;
    vec2 uvCoord;
    if (abst.x > abst.y && abst.x > abst.z)
    {
        // x major
        float negx = step(texCoord.x, 0.0);
        uvCoord = mix(-texCoord.zy, vec2(texCoord.z, -texCoord.y), negx);
        cubeFace = negx;
    }
    else if (abst.y > abst.z)
    {
        // y major
        float negy = step(texCoord.y, 0.0);
        uvCoord = mix(texCoord.xz, vec2(texCoord.x, -texCoord.z), negy);
        cubeFace = 2.0 + negy;
    }
    else
    {
        // z major
        float negz = step(texCoord.z, 0.0);
        uvCoord = mix(vec2(texCoord.x, -texCoord.y), -texCoord.xy, negz);
        cubeFace = 4.0 + negz;
    }
    uvCoord = (uvCoord + 1.0) * 0.5; // 0..1
    uvCoord = uvCoord * cubemapSize;
    uvCoord = clamp(uvCoord, vec2(0.0), cubemapSize - vec2(1.0));

    return ivec3(ivec2(uvCoord), int(cubeFace));
}

