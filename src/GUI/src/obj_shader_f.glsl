#version 330 core
in vec3 M_normal;
in vec3 M_pos;

void main()
{
    // Ambient
    vec4 M_AmbientLightColor = vec4(0.4, 0.4, 0.4, 1.0);
    vec4 M_AmbientMaterial = vec4(0.2, 0.2, 0.2, 1.0);
    vec4 ambientColor = M_AmbientLightColor * M_AmbientMaterial;

    // Diffuse
    vec3 M_LightPos = vec3(-10.0, -10.0, 0.0);
    vec3 Light = normalize(M_LightPos);       // 指向光源的单位向量
    vec3 Normal = normalize(M_normal);      //  法线的单位向量

    // 点乘获取光照强度
    vec4 M_DiffuseLightColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 M_DiffuseMaterial = vec4(0.9, 0.9, 0.9, 1.0);
    vec4 diffuseColor = M_DiffuseLightColor * M_DiffuseMaterial * max(0.0, dot(Normal, Light));

    // 镜面反射
    vec4 specularLightColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 specularMaterial = vec4(0.4, 0.4, 0.4, 1.0);
    vec3 reflerDir = normalize(reflect(-Light, Normal));
    vec3 eyeDir = normalize(vec3(0.0) - M_pos);
    vec4 specularColor = specularLightColor * specularMaterial * pow(max(0.0, dot(reflerDir, eyeDir)), 180);

    gl_FragColor = ambientColor + diffuseColor + specularColor;
    //gl_FragColor = vec4(0.6,0.6,0.6,0.6);
}