#pragma once

namespace cgtub
{

constexpr const char* vertex_shader_source = R"glsl(
    #version 330

    precision highp float;

    uniform mat4 model_view_matrix;
    uniform mat4 projection_matrix;

    in vec3 position_vs;

    void main()
    {
        gl_Position = projection_matrix * model_view_matrix * vec4(position_vs, 1.);
    }
)glsl";

constexpr const char* fragment_shader_source = R"glsl(
    #version 330
    
    precision highp float;

    uniform vec3 color;

    out vec4 f_color;

    void main()
    {
        f_color = vec4(color, 1.0);
    }
)glsl";

constexpr const char* colorlit_vertex_shader_source = R"glsl(
    #version 330

    precision highp float;

    uniform mat4 model_matrix;
    uniform mat4 model_view_matrix;
    uniform mat4 projection_matrix;

    in vec3 position_vs;

    out vec3 position_view;

    void main()
    {
        position_view = (model_matrix * vec4(position_vs, 1.)).xyz;
        gl_Position   = projection_matrix * model_view_matrix * vec4(position_vs, 1.);
    }
)glsl";

constexpr const char* colorlit_fragment_shader_source = R"glsl(
    #version 330
    
    precision highp float;

    uniform vec3 color;

    in vec3 position_view;

    out vec4 f_color;

    void main()
    {
        vec3 dpdx = dFdx(position_view);
        vec3 dpdy = dFdy(position_view);
        vec3 normal = normalize(cross(dpdx, dpdy));

        float l = 0.3*max(normal.y + 0.4*normal.x, 0.f) + 
                  0.1*max(normal.y + -0.3*normal.z - 0.4*normal.x, 0.f) + 
                  0.2*max(normal.z, 0.f);
        float b = 0.05f;

        f_color = vec4(color * (1.5f * l + b), 1.0);
    }
)glsl";

constexpr const char* vcolor_vertex_shader_source = R"glsl(
    #version 330

    precision highp float;

    uniform mat4 model_view_matrix;
    uniform mat4 projection_matrix;

    in vec3 position_vs;
    in vec3 color_vs;

    out vec3 color_fs;
        
    void main()
    {
        color_fs    = color_vs;
        gl_Position = projection_matrix * model_view_matrix * vec4(position_vs, 1.);
    }
)glsl";

constexpr const char* vcolor_fragment_shader_source = R"glsl(
    #version 330
    
    precision highp float;

    in vec3 color_fs;

    out vec4 color_frag;

    void main()
    {
        color_frag = vec4(color_fs, 1.0);
    }
)glsl";

constexpr const char* position_vertex_shader_source = R"glsl(
    #version 330

    precision highp float;

    uniform mat4 model_matrix;
    uniform mat4 model_view_matrix;
    uniform mat4 projection_matrix;

    in vec3 position_vs;

    out vec3 color_fs;
        
    void main()
    {
        color_fs    = 0.5*((model_matrix * vec4(position_vs, 1.)).xyz + 1.0);
        gl_Position = projection_matrix * model_view_matrix * vec4(position_vs, 1.);
    }
)glsl";


constexpr const char* identifier_fragment_shader_source = R"glsl(
    #version 330
    
    precision highp float;

    uniform int id;

    out vec4 f_color;

    // https://stackoverflow.com/a/74419913
    float random(vec2 st)
    {
        return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
    }

    void main()
    {
        vec3 color = vec3(id, random(vec2(id, 1)), random(vec2(id, 2)));
        f_color = vec4(color, 1.0);
    }
)glsl";

} // namespace cgtub