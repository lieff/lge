
inline float dot(const vec2 &v1, const vec2 &v2)
{
    return v1.x*v2.x + v1.y*v2.y;
}

inline float dot(const vec3 &v1, const vec3 &v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

inline vec2 max(const vec2 &v1, const vec2 &v2)
{
    return glm::max(v1, v2);
}

inline vec3 max(const vec3 &v1, const vec3 &v2)
{
    return glm::max(v1, v2);
}

inline float max(const float v1, const float v2)
{
    return fmax(v1, v2);
}

inline void operator += (vec2 &v, const float v1)
{
    v += vec2(v1);
}

inline void operator += (vec3 &v, const float v1)
{
    v += vec3(v1);
}

inline const vec2 operator + (const vec2 &v1, const float v2)
{
    return v1 + vec2(v2);
}

inline const vec2 operator - (const vec2 &v1, const float v2)
{
    return v1 - vec2(v2);
}

inline const vec2 operator * (const vec2 &v1, const float v2)
{
    return v1 * vec2(v2);
}

inline const vec3 operator * (const vec3 &v1, const float v2)
{
    return v1 * vec3(v2);
}

inline const vec4 operator * (const vec4 &v1, const float v2)
{
    return v1 * vec4(v2);
}

inline const vec2 operator / (const vec2 &v1, const float v2)
{
    return v1 / vec2(v2);
}

inline const vec3 operator / (const vec3 &v1, const float v2)
{
    return v1 / vec3(v2);
}

inline const vec4 operator / (const vec4 &v1, const float v2)
{
    return v1 / vec4(v2);
}

inline const vec3 operator + (const vec3 &v1, const float v2)
{
    return v1 + vec3(v2);
}

inline const vec4 operator + (const vec4 &v1, const float v2)
{
    return v1 + vec4(v2);
}

inline const float mod(const float a, const float b)
{
    return a - b * floor(a / b);
}

inline const float step(const float a, const float b)
{
    return (b >= a) ? 1.f : 0.f;
}

inline const float clamp(const float &v, const float a, const float b)
{
    return fmaxf(a, fminf(v, b));
}

inline const float mix(const float &v, const float a, const float b)
{
    return glm::mix(v, a, b);
}

inline const float min(const float a, const float b)
{
    return glm::min(a, b);
}

inline const vec2 clamp(const vec2 &v, const float a, const float b)
{
    return clamp(v, vec2(a), vec2(b));
}

inline float smoothstep(float edge0, float edge1, float x)
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}
