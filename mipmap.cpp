#include "framework.h"

using namespace prototyper;

void set_workgroup_size( vec2& gws, vec2& lws, vec2& dispatch_size, const uvec2& screen )
{
  //set up work group sizes
  unsigned local_ws[2] = { 16, 16 };
  unsigned global_ws[2];
  unsigned gw = 0, gh = 0, count = 1;

  while( gw < screen.x )
  {
    gw = local_ws[0] * count;
    count++;
  }

  count = 1;

  while( gh < screen.y )
  {
    gh = local_ws[1] * count;
    count++;
  }

  global_ws[0] = gw;
  global_ws[1] = gh;

  gws = vec2( global_ws[0], global_ws[1] );
  lws = vec2( local_ws[0], local_ws[1] );
  dispatch_size = gws / lws;
}

void gen_mipmaps( GLuint shader, GLuint texture, GLenum internal_format, const uvec2& size, unsigned miplevels )
{
  glUseProgram( shader );

  uvec2 s = size;

  s.x /= 2;
  s.y /= 2;

  for( int d = 1; d < miplevels; ++d )
  {
    glBindImageTexture( 0, texture, d - 1, GL_FALSE, 0, GL_READ_ONLY, internal_format );
    glBindImageTexture( 1, texture, d, GL_FALSE, 0, GL_WRITE_ONLY, internal_format );

    vec2 dispatch_size, gws, lws;
    set_workgroup_size( gws, lws, dispatch_size, s );

    glDispatchCompute( dispatch_size.x, dispatch_size.y, 1 );

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

    s.x /= 2;
    s.y /= 2;
  }
}

int main( int argc, char** argv )
{
  uvec2 screen( 1280, 720 );
  bool fullscreen = false;
  bool silent = false;
  string title = "Image Mipmap generation";

  /*
   * Initialize the OpenGL context
   */

  framework frm;
  frm.init( screen, title, fullscreen );
  //frm.set_vsync( true );

  //set opengl settings
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glFrontFace( GL_CCW );
  glEnable( GL_CULL_FACE );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClearDepth( 1.0f );

  frm.get_opengl_error();

  /*
   * Set up the shaders
   */

  GLuint downsample_shader = 0;
  frm.load_shader( downsample_shader, GL_COMPUTE_SHADER, "../shaders/mipmap/downsample.cs" );

  GLuint display_shader = 0;
  frm.load_shader( display_shader, GL_VERTEX_SHADER, "../shaders/mipmap/display.vs" );
  frm.load_shader( display_shader, GL_FRAGMENT_SHADER, "../shaders/mipmap/display.ps" );

  /*
   * Create mipmap levels
   */

  /**/

  GLuint ss_quad = frm.create_quad( vec3( -1, -1, 0 ), vec3( 1, -1, 0 ), vec3( -1, 1, 0 ), vec3( 1, 1, 0 ) );

  sf::Image im;
  im.loadFromFile( "../resources/image.jpg" );

  uvec2 size = uvec2( im.getSize().x, im.getSize().y );
  GLenum internal_format = GL_RGBA8;

  unsigned miplevels = std::log2( float( std::max( size.x, size.y ) ) ) + 1;

  GLuint tex = 0;
  glGenTextures( 1, &tex );

  glBindTexture( GL_TEXTURE_2D, tex );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  //glTexStorage2D( GL_TEXTURE_2D, miplevels, internal_format, size.x, size.y );
  //glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, im.getPixelsPtr() );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, im.getPixelsPtr() );

  glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );

  glGenerateMipmap( GL_TEXTURE_2D );
  //gen_mipmaps( downsample_shader, tex, internal_format, size, miplevels ); 

  //create texview
  /*GLuint texview = 0;
  glGenTextures( 1, &texview );

  glTextureView( texview, GL_TEXTURE_2D, tex, GL_RGBA8, 0, miplevels, 0, 1 );
  glBindTexture( GL_TEXTURE_2D, texview );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);*/

  /**/

  /*
   * Handle events
   */

  float lod = 0;

  bool default_or_mine = false; //default

  auto event_handler = [&]( const sf::Event & ev )
  {
    switch( ev.type )
    {
      case sf::Event::KeyPressed:
      {
                                  if( ev.key.code == sf::Keyboard::Add )
                                  {
                                    if( lod < 10 )
                                    {
                                      lod += 0.1f;
                                    }
                                  }

                                  if( ev.key.code == sf::Keyboard::Subtract )
                                  {
                                    if( lod > 0 )
                                    {
                                      lod -= 0.1f;
                                    }
                                  }

                                  if( ev.key.code == sf::Keyboard::Space )
                                  {
                                    default_or_mine = !default_or_mine;
                                  }
      }
      default:
        break;
    }
  };

  /*
   * Render
   */

  sf::Clock timer;
  timer.restart();

  frm.display( [&]
  {
    frm.handle_events( event_handler );

    glClearColor( 1, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glDisable( GL_DEPTH_TEST );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glViewport( 0, 0, screen.x, screen.y );

    if( default_or_mine )
    {
      gen_mipmaps( downsample_shader, tex, internal_format, size, miplevels );
      frm.set_title( "mine " + std::to_string( int( lod ) + 1 ) );
    }
    else
    {
      glGenerateMipmap( GL_TEXTURE_2D );
      frm.set_title( "default " + std::to_string( int( lod ) + 1 ) );
    }

    glUseProgram( display_shader );

    glUniform1f( 0, lod );

    glActiveTexture( GL_TEXTURE0 );
    //glBindTexture( GL_TEXTURE_2D, texview );
    glBindTexture( GL_TEXTURE_2D, tex );

    glBindVertexArray( ss_quad );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

    frm.get_opengl_error();
  }, silent );

  return 0;
}
