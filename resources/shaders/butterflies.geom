#version 450

layout( points ) in;
layout( triangle_strip, max_vertices = 8 ) out;

layout( binding = 0 ) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 proj;
  float time;
  int numPrimitives;
};

layout(location = 0) out vec2 TexCoord;

vec2 WingSize = vec2(1.0, 2.0);

void main( )
{
    mat4 modelView = view * model;

    /* Alpha angle is the wings aperture angle. This will give us a rotation 
    in the range [5, 85] degrees */
    
    // gl_PrimitiveIDIn is the index of the current primitive that is being processed.
    float alpha = radians(5.0 + (gl_PrimitiveIDIn / (numPrimitives - 1.0)) * 
        80.0 * sin(time));
    
    // Beta angle is the orientation of the butterfly
    float beta = radians(-90.0f + (gl_PrimitiveIDIn / (numPrimitives - 1.0)) * 90.0);
    
    // Matrix to translate the wing to the origin
    mat4 T = mat4(vec4( 1.0, 0.0, 0.0, 0.0 ),
                                vec4( 0.0, 1.0, 0.0, 0.0 ),
                                vec4( 0.0, 0.0, 1.0, 0.0 ),
                                vec4( -gl_in[0].gl_Position.xyz, 1.0 ));

    // Matrix to translate the wing back to its original position
    mat4 Ti = mat4(vec4( 1.0, 0.0, 0.0, 0.0 ),
                                 vec4( 0.0, 1.0, 0.0, 0.0 ),
                                 vec4( 0.0, 0.0, 1.0, 0.0 ),
                                 vec4( gl_in[0].gl_Position.xyz, 1.0));
    /* Matrix to rotate the whole butterfly to change its flying direction. This is a Z axis rotation matrix. */
    mat4 Rz = mat4(vec4( cos(beta), sin(beta), 0.0, 0.0 ),
                                 vec4( -sin(beta), cos(beta), 0.0, 0.0 ),
                                 vec4( 0.0, 0.0, 1.0, 0.0 ),
                                 vec4( 0.0, 0.0, 0.0, 1.0 ));

    // Left wing creation.
    // Matrix to rotate the left wing and give an appearance of moving wings. This is a Y axis rotation matrix
    mat4 Ry = mat4(vec4(cos(alpha), 0, -sin(alpha), 0),
                                 vec4(0, 1, 0, 0),
                                 vec4(sin(alpha), 0, cos(alpha), 0),
                                 vec4(0, 0, 0, 1));
    // Final transform matrix
    mat4 M = proj * modelView * Rz * Ti * Ry * T;

    // Using the original point's position and the wing's size, we calculate each new vertex. Then we transform it with the accumulated matrix (M)

        // 1st vertex
        gl_Position = M * vec4(
            gl_in[0].gl_Position.x, 
            gl_in[0].gl_Position.y - WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        // Create a proper texture coordinate for this vertex
        TexCoord = vec2( 1.0, 0.0 );
        EmitVertex( );

        // 2nd vertex
        gl_Position = M * vec4(
            gl_in[0].gl_Position.x, 
            gl_in[0].gl_Position.y + WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        TexCoord = vec2( 1.0, 1.0 );
        EmitVertex( );

        // 3rd vertex
        gl_Position = M * vec4(
            gl_in[0].gl_Position.x - WingSize.x, 
            gl_in[0].gl_Position.y - WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        TexCoord = vec2( 0.0, 0.0 );
        EmitVertex( );

        //4rd vertex
        gl_Position = M * vec4(
            gl_in[0].gl_Position.x - WingSize.x, 
            gl_in[0].gl_Position.y + WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        TexCoord = vec2( 0.0, 1.0 );
        EmitVertex( );

    EndPrimitive( );

    // The aperture angle for the right wing is 180 - alpha
        alpha = 3.141592f - alpha;
        Ry = mat4(vec4(cos(alpha), 0, -sin(alpha), 0),
        vec4(0, 1, 0, 0),
        vec4(sin(alpha), 0, cos(alpha), 0),
        vec4(0, 0, 0, 1));
        M = proj * modelView * Rz * Ti * Ry * T;

        gl_Position = M * vec4(
            gl_in[0].gl_Position.x + WingSize.x, 
            gl_in[0].gl_Position.y - WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        TexCoord = vec2( 0.0, 0.0 );
        EmitVertex();
        
        gl_Position = M * vec4(
            gl_in[0].gl_Position.x + WingSize.x, 
            gl_in[0].gl_Position.y + WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        TexCoord = vec2( 0.0, 1.0 );
        EmitVertex();
        
        gl_Position = M * vec4(
            gl_in[0].gl_Position.x, 
            gl_in[0].gl_Position.y - WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        TexCoord = vec2( 1.0, 0.0 );
        EmitVertex();
        
        gl_Position = M * vec4(
            gl_in[0].gl_Position.x, 
            gl_in[0].gl_Position.y + WingSize.y * 0.5, 
            gl_in[0].gl_Position.zw
        );
        TexCoord = vec2( 1.0, 1.0 );
        EmitVertex();
    EndPrimitive();
}