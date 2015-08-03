#version 430 core

layout(binding=0, rgba8) readonly uniform image2D src_tex;
layout(binding=1) writeonly uniform image2D dst_tex;

layout(local_size_x = 16, local_size_y = 16) in; //local workgroup size

vec4 sample_point( vec2 coord, vec2 size )
{
  return imageLoad( src_tex, ivec2(coord * size) );
}

vec4 sample_bilinear( vec2 coord, vec2 size )
{
  ivec2 final_coord = ivec2(coord * size);
  vec4 s00 = imageLoad( src_tex, final_coord + ivec2(0, 0) );
  vec4 s01 = imageLoad( src_tex, final_coord + ivec2(1, 0) );
  vec4 s10 = imageLoad( src_tex, final_coord + ivec2(0, 1) );
  vec4 s11 = imageLoad( src_tex, final_coord + ivec2(1, 1) );
  
  float xval = fract( coord.x * size.x );
  float yval = fract( coord.y * size.y );
  
  return mix( mix( s00, s10, xval ), mix( s01, s11, xval ), yval );
}

float triangular( float f )
{
  f *= 0.5;
  return f < 0 ? f + 1 : 1 - f;
}

float bell( float f )
{
  f = f * (0.5 * 1.5); //rescale [-2...2] to [-1.5...1.5]
  
  if( f >= -1.5 && f < -0.5 )
  {
    f += 1.5;
    return 0.5 * f * f;
  }
  else if( f >= -0.5 && f < 0.5 )
  {
    return 0.75 - ( f * f );
  }
  else if( f >= 0.5 && f < 1.5 )
  {
    f -= 1.5;
    return 0.5 * f * f;
  }
  else
  {
    return 0;
  }
}

float bspline( float f )
{
  f = abs(f);
  
  if( f >= 0 && f <= 1 )
  {
    return (2/3.0) + 0.5 * ( f * f * f ) - ( f * f );
  }
  else if( f > 1 && f <= 2 )
  {
    f = 2 - f;
    return (1/6.0) * f * f * f;
  }
  else
  {
    return 1;
  }
}

vec4 sample_bicubic( vec2 coord, vec2 size )
{
  vec4 sum = vec4(0);
  vec4 weight_sum = vec4(0);
  
  ivec2 final_coord = ivec2(coord * size);
  
  float xval = fract( coord.x * size.x );
  float yval = fract( coord.y * size.y );
  
  for( int y = -1; y <= 2; ++y )
  {
    for( int x = -1; x <= 2; ++x )
    {
      vec4 s = imageLoad( src_tex, final_coord + ivec2(x, y) );
      float fx = bspline( x - xval ); //can use bell or triangular as functions
      float fy = bspline( -(y - yval) );
      float weight = fx * fy;
      sum += s * weight;
      weight_sum += weight;
    }
  }
  
  return sum / weight_sum;
}

void main()
{
	ivec2 global_id = ivec2( gl_GlobalInvocationID.xy );
  ivec2 global_size = imageSize( dst_tex ).xy;
  ivec2 src_size = imageSize( src_tex ).xy;
  vec2 texcoord = vec2(global_id) / vec2(global_size);
  
  if( global_id.x <= global_size.x && global_id.y <= global_size.y )
	{  
    //point filtering
    //imageStore( dst_tex, global_id, sample_point( texcoord, src_size ) );
    
    //bilinear filtering
    //imageStore( dst_tex, global_id, sample_bilinear( texcoord, src_size ) );
    
    //bicubic filtering
    imageStore( dst_tex, global_id, sample_bicubic( texcoord, src_size ) );
  }
}