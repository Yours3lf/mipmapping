#version 430 core

layout(binding=0, rgba8) readonly uniform image2D src_tex;
layout(binding=1) writeonly uniform image2D dst_tex;

layout(local_size_x = 16, local_size_y = 16) in; //local workgroup size

vec4 sample_box( vec2 coord, vec2 src_size, vec2 dst_size )
{
  ivec2 bottom_left_pixel = ivec2(coord * src_size);

  float sample_count = 0;
  vec4 sample_sum = vec4(0);

  ivec2 kernel_size = ivec2(src_size / dst_size);

  for(int y = 0; y < kernel_size.y; ++y)
    for(int x = 0; x < kernel_size.x; ++x)
    {
      sample_sum += imageLoad(src_tex, bottom_left_pixel + ivec2(x,y));
      sample_count++;
    }

  return sample_sum / sample_count;
}

void main()
{
	ivec2 global_id = ivec2( gl_GlobalInvocationID.xy );
  ivec2 global_size = imageSize( dst_tex ).xy;
  ivec2 src_size = imageSize( src_tex ).xy;
  vec2 texcoord = vec2(global_id) / vec2(global_size);

  if( global_id.x <= global_size.x && global_id.y <= global_size.y )
	{
    //box filter sampling
    imageStore( dst_tex, global_id, sample_box( texcoord, src_size, global_size ) );
  }
}
