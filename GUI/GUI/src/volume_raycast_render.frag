#version 430 core
out vec4 oFragColor;

uniform sampler3D VolumeData;
uniform sampler2DRect  RayStartPos;
uniform sampler2DRect  RayEndPos;
uniform sampler1D TransferFunc;
uniform float voxel;
uniform float step;
uniform vec3 volume_start_pos;
uniform vec3 volume_extend;
uniform vec4 bg_color;
uniform mat4 MVPMatrix;
uniform vec4 camera_pos;
uniform vec3 padding;
uniform vec2 click_coord;
layout(std430,binding = 0) buffer PointInfo{
    vec4 info[];
}point_info;
vec3 GetSamplePos(in vec3 ray_pos){
    return (ray_pos - volume_start_pos + padding) / volume_extend;
}
vec3 PhongShading(in vec3 pos,in vec3 diffuse_color,in vec3 ray_direction){
    vec3 N;
    float x1,x2;
    int count = 0;
    x1 = texture(VolumeData,GetSamplePos(pos + vec3(voxel,0.0,0.0))).r;
    x2 = texture(VolumeData,GetSamplePos(pos - vec3(voxel,0.0,0.0))).r;
    N.x = x1 -x2;
    x1 = texture(VolumeData,GetSamplePos(pos + vec3(0.0,voxel,0.0))).r;
    x2 = texture(VolumeData,GetSamplePos(pos - vec3(0.0,voxel,0.0))).r;
    N.y = x1 -x2;
    x1 = texture(VolumeData,GetSamplePos(pos + vec3(0.0,0.0,voxel))).r;
    x2 = texture(VolumeData,GetSamplePos(pos - vec3(0.0,0.0,voxel))).r;
    N.z = x1 -x2;
    N = - normalize(N);
    vec3 L = -ray_direction;
    vec3 R = L;
    vec3 ambient = 0.05 * diffuse_color;
    vec3 diffuse = diffuse_color * max(dot(N,L),0.0);
    vec3 specular = 0.2 * pow(max(dot(N,(N+R)*0.5),0.0),64) * vec3(1.0);
    vec3 radiance = ambient + diffuse + specular;
    return pow(radiance,vec3(1.0/2.2));
}
void main(){
    vec3 start_pos;
    if(camera_pos.w == 1.0){
        start_pos = camera_pos.xyz;
    }
    else
        start_pos = texture2DRect(RayStartPos,ivec2(gl_FragCoord)).xyz;
    vec3 end_pos = texture2DRect(RayEndPos,ivec2(gl_FragCoord)).xyz;

    if(end_pos == vec3(0.0)){
        end_pos = start_pos;
    }

    vec3 route = end_pos - start_pos;
    if(length(route) < 0.001){
        discard;
    }
    vec3 ray_direction = normalize(route);
    ray_direction = normalize(ray_direction );

    float dist = dot(ray_direction,route);

    int steps = int(dist / step);
    vec3 ray_pos = start_pos;
    vec4 color = vec4(0.0);
    int i = 0;
    float max_scalar = 0.0;

    bool query = false;
    if(ivec2(click_coord) == ivec2(gl_FragCoord)){
        query = true;
    }
    for(; i < steps; i++){
        vec3 sample_pos = GetSamplePos(ray_pos);

        float scalar = texture(VolumeData,sample_pos).r;
        max_scalar = max(max_scalar,scalar);
        if(scalar > 0.25){

//            vec4 sample_color = texture(TransferFunc,scalar);
            vec4 sample_color = vec4(scalar);

//            sample_color.rgb = PhongShading(ray_pos,sample_color.rgb,ray_direction);
            color = color + sample_color * vec4(sample_color.aaa,1.0)*(1.0 - color.a);
            if(color.a > 0.99)
                break;
        }
        ray_pos = start_pos + i * ray_direction * step;
    }
    vec4 p = MVPMatrix * vec4(ray_pos,1.0);
    gl_FragDepth = (p.z / p.w) * 0.5 + 0.5;
    if(query){
        ray_pos = start_pos;
        vec3 start = vec3(0.0);
        vec3 end = vec3(0.0);
        vec3 max_pos = vec3(0.0);
        float r = 0.0;
        for(int i = 0;i<steps;i++){
            vec3 sample_pos = GetSamplePos(ray_pos);
            float scalar = texture(VolumeData,sample_pos).r;
            if(scalar == max_scalar){
                max_pos = ray_pos;
                break;
            }
            ray_pos = start_pos + i * ray_direction * step;
        }
        ray_pos = max_pos;
        start_pos = max_pos;
        for(int i = 0;i<steps;i++){
            vec3 sample_pos = GetSamplePos(ray_pos);
            float scalar = texture(VolumeData,sample_pos).r;
            if(scalar < 0.25){
                start = ray_pos;
                break;
            }
            ray_pos = start_pos - i * ray_direction * step;
        }
        ray_pos = max_pos;
        for(int i = 0;i<steps;i++){
            vec3 sample_pos = GetSamplePos(ray_pos);
            float scalar = texture(VolumeData,sample_pos).r;
            if(scalar < 0.25){
                end = ray_pos;
                break;
            }
            ray_pos = start_pos + i * ray_direction * step;
        }
        r = distance(start,end) * 0.5;
        point_info.info[0] = vec4(start,max_scalar);
        point_info.info[1] = vec4(end,r);
    }
    if(color.a == 0.0)
        discard;
    color = (1.0 - color.a) * bg_color + color * color.a;
    color.a = 1.0;
    oFragColor = color;
}